#include "ks_planificador_corto.h"

#include "../main/ks_main.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../mensajes/ks_mensajes.h"

void *ks_planificador_corto_plazo(void *arg)
{
	(void)arg;

	while (1)
	{
		sem_wait(&sem_ready);
		sem_wait(&sem_cpu_libre);

		// No se puede planificar durante una compactación (KM está moviendo la
		// memoria): devuelvo los tokens que tomé y espero el FIN_COMPACTACION.
		// Chequeo DESPUÉS de tomar los sem porque la compactación pudo arrancar
		// mientras esperaba en sem_ready / sem_cpu_libre.
		pthread_mutex_lock(&mutex_compactacion);
		if (compactacion_en_progreso)
		{
			pthread_mutex_unlock(&mutex_compactacion);
			sem_post(&sem_cpu_libre);
			sem_post(&sem_ready);
			sem_wait(&sem_compactacion_finalizada); // se libera en FIN_COMPACTACION
			continue;
		}
		pthread_mutex_unlock(&mutex_compactacion);

		// Primero los desalojados por compactación (ready prioritario). En FIFO/RR
		// es indistinto; en CMN garantiza que se replanifiquen antes que el resto.
		t_pcb *pcb = ks_obtener_siguiente_ready_prioritario();
		if (pcb == NULL)
		{
			pcb = ks_obtener_siguiente_ready();
		}

		if (pcb == NULL)
		{
			sem_post(&sem_cpu_libre);
			continue;
		}

		switch (ks_config.algoritmo)
		{
		case FIFO:
			ks_planificar_fifo(pcb);
			break;

		case RR:
			ks_planificar_rr(pcb);
			break;

		case CMN:
			ks_planificar_cmn(pcb);
			break;

		default:
			break;
		}
	}

	return NULL;
}

t_pcb *ks_obtener_siguiente_ready(void)
{
	for (int i = 0; i < cantidad_algoritmos; i++)
	{
		pthread_mutex_lock(&array_queues[i].mutex_ready);

		if (!queue_is_empty(array_queues[i].queue_ready))
		{
			t_pcb *pcb = queue_pop(array_queues[i].queue_ready);
			pthread_mutex_unlock(&array_queues[i].mutex_ready);

			return pcb;
		}

		pthread_mutex_unlock(&array_queues[i].mutex_ready);
	}

	return NULL;
}

t_pcb *ks_obtener_siguiente_ready_prioritario(void)
{
	for (int i = 0; i < cantidad_algoritmos; i++)
	{
		pthread_mutex_lock(&array_queues[i].mutex_ready_prioritario);

		if (!queue_is_empty(array_queues[i].queue_ready_prioritario))
		{
			t_pcb *pcb = queue_pop(array_queues[i].queue_ready_prioritario);
			pthread_mutex_unlock(&array_queues[i].mutex_ready_prioritario);

			return pcb;
		}
		pthread_mutex_unlock(&array_queues[i].mutex_ready_prioritario);
	}

	return NULL;
}

t_cpu *ks_enviar_a_exec(t_pcb *pcb)
{
	// buscar una CPU libre y asignarle el proceso
	pthread_mutex_lock(&mutex_cpus);
	t_cpu *cpu_libre = NULL;
	for (int i = 0; i < list_size(cpus); i++)
	{
		t_cpu *cpu = list_get(cpus, i);
		if (cpu->pcb_ejecutando == NULL)
		{
			cpu_libre = cpu;
			break;
		}
	}
	if (cpu_libre != NULL)
	{
		cpu_libre->pcb_ejecutando = pcb;
		pcb->cpu_fd = cpu_libre->fd;
	}
	pthread_mutex_unlock(&mutex_cpus);

	if (cpu_libre == NULL)
	{
		// no debería pasar (el corto plazo esperó sem_cpu_libre): devolverlo a ready prioritario
		log_error(ks_logger, "ks_enviar_a_exec: no hay CPU libre para el PID %d, vuelve a READY", pcb->pid);
		pcb->estado = READY;
		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);
		queue_push(array_queues[ks_indice_cola(pcb->prioridad)].queue_ready_prioritario, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);
		sem_post(&sem_ready);
		return NULL;
	}

	pcb->estado = EXEC;

	pthread_mutex_lock(&mutex_exec);
	list_add(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	enviar_alojar_proceso_cpu(pcb); // ALOJAR_PROCESO [pid] -> CPU

	log_cambio_estado(pcb, READY, EXEC);

	return cpu_libre;
}

void ks_planificar_fifo(t_pcb *pcb)
{
	ks_enviar_a_exec(pcb);
}

// Timer de quantum de RR. Corre en un hilo propio para NO bloquear el planificador.
// Al vencer el quantum, si el proceso SIGUE ejecutando en la MISMA CPU, la interrumpe.
// El desalojo real (EXEC→READY normal + liberar CPU) lo hace el handler DESALOJO_PROCESO
// cuando la CPU responde a la INTERRUPCION (mismo camino que el desalojo por prioridad).
typedef struct
{
	t_pcb *pcb;
	t_cpu *cpu;
} t_timer_rr_args;

static void *ks_timer_quantum(void *arg)
{
	t_timer_rr_args *args = arg;
	t_pcb *pcb = args->pcb;
	t_cpu *cpu = args->cpu;
	free(args);

	usleep(ks_config.rr_quantum * 1000); // RR_QUANTUM viene en ms, usleep toma µs

	// ¿Sigue el MISMO proceso en esta CPU? (pudo bloquearse, terminar o ser desalojado).
	// Los PCB viven hasta el shutdown, así que la comparación de punteros es segura.
	// Chequeo Y envío bajo mutex_cpus: ks_pasar_a_block también toma ese mutex para
	// poner pcb_ejecutando=NULL, así el bloqueo no se cuela entre el chequeo y el envío.
	pthread_mutex_lock(&mutex_cpus);
	if (cpu->pcb_ejecutando == pcb)
	{
		log_desalojo_quantum(pcb);
		enviar_interrupcion_cpu(cpu->fd, pcb->pid);
	}
	pthread_mutex_unlock(&mutex_cpus);

	return NULL;
}

void ks_planificar_rr(t_pcb *pcb)
{
	t_cpu *cpu = ks_enviar_a_exec(pcb);
	if (cpu == NULL)
	{
		return; // no se pudo despachar (volvió a READY): sin quantum
	}

	// El quantum se cuenta en un hilo aparte; al vencer interrumpe la CPU si el
	// proceso todavía está ahí. Así el planificador queda libre para seguir.
	t_timer_rr_args *args = malloc(sizeof(t_timer_rr_args));
	args->pcb = pcb;
	args->cpu = cpu;

	pthread_t hilo_quantum;
	pthread_create(&hilo_quantum, NULL, ks_timer_quantum, args);
	pthread_detach(hilo_quantum);
}

void ks_planificar_cmn(t_pcb *pcb)
{

	// pthread_mutex_lock(&mutex_array_queues);
	algoritmos alg = array_queues[ks_indice_cola(pcb->prioridad)].alg;
	// pthread_mutex_unlock(&mutex_array_queues);

	switch (alg)
	{
	case FIFO:
		ks_planificar_fifo(pcb);
		break;

	case RR:
		ks_planificar_rr(pcb);
		break;

	default:
		break;
	}
}

void ks_verificar_desalojo_prioridad(t_pcb *nuevo)
{
	// El desalojo entre colas es exclusivo de CMN: en FIFO/RR la prioridad no tiene
	// injerencia (spec). Además debe estar habilitado por config.
	if (ks_config.algoritmo != CMN || !ks_config.queue_preemption)
	{
		return;
	}

	pthread_mutex_lock(&mutex_exec);

	// Elegir el proceso en ejecución MENOS prioritario (mayor número de prioridad)
	// que además sea menos prioritario que el que llegó: es a ese al que hay que
	// desalojar para darle lugar (spec: "desalojar al Proceso menos prioritario").
	t_pcb *victima = NULL;
	for (int i = 0; i < list_size(procs_exec); i++)
	{
		t_pcb *exec = list_get(procs_exec, i);
		if (exec->prioridad > nuevo->prioridad && (victima == NULL || exec->prioridad > victima->prioridad))
		{
			victima = exec;
		}
	}

	if (victima != NULL)
	{
		enviar_interrupcion_cpu(victima->cpu_fd, victima->pid);
		log_desalojo_prioridad(victima, nuevo);
		// El desalojo real (EXEC→READY normal + liberar CPU) lo hace el handler
		// DESALOJO_PROCESO cuando la CPU responda a la INTERRUPCION.
	}

	pthread_mutex_unlock(&mutex_exec);
}

void ks_empieza_a_desalojar_por_compactacion(int pid_disparador)
{
	pthread_mutex_lock(&mutex_exec);

	// El proceso que disparó la compactación (MEM_ALLOC) está en procs_exec pero su CPU
	// ya lo liberó por la syscall y quedó idle: NO se lo interrumpe (no respondería el
	// DESALOJO_PROCESO -> el contador nunca llegaría a 0 -> deadlock con KM). Se cuenta e
	// interrumpe solo al resto, que sí está ejecutando de verdad.
	t_list *a_desalojar = list_create();
	for (int i = 0; i < list_size(procs_exec); i++)
	{
		t_pcb *pcb = list_get(procs_exec, i);
		if (pcb->pid != pid_disparador)
		{
			list_add(a_desalojar, pcb);
		}
	}
	int cantidad_a_desalojar = list_size(a_desalojar);

	// Fijo el contador ANTES de interrumpir: cada CPU responderá DESALOJO_PROCESO
	// (en otro hilo) y decrementará; al llegar a 0 se confirma a KM.
	pthread_mutex_lock(&mutex_compactacion);
	cpus_pendientes_desalojo_compactacion = cantidad_a_desalojar;
	pthread_mutex_unlock(&mutex_compactacion);

	for (int i = 0; i < cantidad_a_desalojar; i++)
	{
		t_pcb *pcb = list_get(a_desalojar, i);

		enviar_interrupcion_compactacion_cpu(pcb->cpu_fd, pcb->pid);
	}
	list_destroy(a_desalojar);

	pthread_mutex_unlock(&mutex_exec);

	// Si no había ninguna CPU ejecutando (salvo el disparador), no llegará ningún
	// DESALOJO_PROCESO: confirmo de inmediato para que KM no quede esperando eternamente.
	if (cantidad_a_desalojar == 0)
	{
		enviar_confirmacion_desalojo_compactacion_km();
	}
}

// Llamada cuando una CPU confirmó su desalojo dentro de una compactación.
// Solo avisa a KM cuando la ÚLTIMA CPU pendiente confirma (transición >0 -> 0),
// así nunca manda la confirmación por duplicado.
void ks_confirmar_desalojo_compactacion_de_una_cpu(void)
{
	bool listo = false;

	pthread_mutex_lock(&mutex_compactacion);
	if (cpus_pendientes_desalojo_compactacion > 0)
	{
		cpus_pendientes_desalojo_compactacion--;
		listo = (cpus_pendientes_desalojo_compactacion == 0);
	}
	pthread_mutex_unlock(&mutex_compactacion);

	if (listo)
	{
		enviar_confirmacion_desalojo_compactacion_km();
	}
}

// Reubica un pcb entre colas de ready cuando cambió su prioridad (herencia).
// Solo actúa si está en READY: en EXEC/BLOCK/SUSP alcanza con haber cambiado el
// campo prioridad, porque al (re)encolarse se usa pcb->prioridad.
void ks_reubicar_ready_por_cambio_prioridad(t_pcb *pcb, int prioridad_vieja, int prioridad_nueva)
{
	if (prioridad_vieja == prioridad_nueva || pcb->estado != READY)
	{
		return;
	}

	pthread_mutex_lock(&array_queues[ks_indice_cola(prioridad_vieja)].mutex_ready);
	bool estaba = list_remove_element(array_queues[ks_indice_cola(prioridad_vieja)].queue_ready->elements, pcb);
	pthread_mutex_unlock(&array_queues[ks_indice_cola(prioridad_vieja)].mutex_ready);

	if (!estaba)
	{
		return; // transicionó justo (ya no estaba en esa cola): nada que reubicar
	}

	pthread_mutex_lock(&array_queues[ks_indice_cola(prioridad_nueva)].mutex_ready);
	queue_push(array_queues[ks_indice_cola(prioridad_nueva)].queue_ready, pcb);
	pthread_mutex_unlock(&array_queues[ks_indice_cola(prioridad_nueva)].mutex_ready);
}

// Desalojo de UN pcb que estaba en EXEC. a_ready_prioritario=true → compactación
// (cola prioritaria, spec); false → desalojo por prioridad (ready normal).
void ks_termina_de_desalojar_un_pcb_general(t_pcb *pcb, bool a_ready_prioritario)
{
	pthread_mutex_lock(&mutex_exec);
	list_remove_element(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	pcb->estado = READY;

	if (a_ready_prioritario)
	{
		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);
		queue_push(array_queues[ks_indice_cola(pcb->prioridad)].queue_ready_prioritario, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);
	}
	else
	{
		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready);
		queue_push(array_queues[ks_indice_cola(pcb->prioridad)].queue_ready, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready);
	}

	log_cambio_estado(pcb, EXEC, READY);
	sem_post(&sem_ready);
}

// (ks_termina_de_desalojar_un_pcb_seg_fault eliminada: el SEG_FAULT ahora FINALIZA
//  el proceso vía finalizar_proceso(pid, "SEG_FAULT"), no lo re-encola en READY.)

void ks_termina_de_desalojar_por_compactacion(void)
{
	pthread_mutex_lock(&mutex_exec);

	for (int i = 0; i < list_size(procs_exec); i++)
	{
		t_pcb *pcb = list_get(procs_exec, i);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);
		queue_push(array_queues[ks_indice_cola(pcb->prioridad)].queue_ready_prioritario, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready_prioritario);

		log_cambio_estado(pcb, EXEC, READY); //  NO HACE FALTA HACER EL SEM_POST PORQUE NO SE PUEDE ALOJAR NADA DURANTE LA COMPACTACIÓN
	}
	// list_clean(procs_exec);

	pthread_mutex_unlock(&mutex_exec);
}
