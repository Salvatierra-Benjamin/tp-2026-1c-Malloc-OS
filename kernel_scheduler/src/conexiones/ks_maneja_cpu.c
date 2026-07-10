#include "ks_maneja_cpu.h"
#include "ks_maneja_io.h"
#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../main/ks_main.h"
#include "../mensajes/ks_mensajes.h"
#include "../planificadores/ks_planificador_corto.h"

t_mutex_kernel *ks_buscar_mutex(char *nombre); // definida al final de este archivo

void *ks_maneja_cpu(void *arg)
{
	t_cpu *cpu = (t_cpu *)arg;
	// free(arg);

	log_debug(ks_logger, "Entra en ks_maneja_cpu (fd = %d)", cpu->fd);
	log_conexion_cpu(cpu->id);

	int ejecutando = 1;

	while (ejecutando)
	{
		int op_code = recibir_operacion(cpu->fd);
		switch (op_code)
		{
		case ENVIO_MUTEX_CREATE: // MUTEX_CREATE (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_CREATE");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			// CPU manda [tam_nombre][nombre con '\0'][pid] (execute_MUTEX_*)
			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int));
			char *nombre_mutex = malloc(len1 + 1);
			buffer_read(nombre_mutex, contenido->buffer, len1 + 1); // el '\0' viene incluido

			t_mutex_kernel *mutex = malloc(sizeof(t_mutex_kernel));

			mutex->nombre = strdup(nombre_mutex);
			mutex->tomado = false;
			mutex->duenio = NULL;
			mutex->bloqueados = queue_create();
			pthread_mutex_init(&mutex->mutex, NULL);

			pthread_mutex_lock(&mutex_mutexes_kernel);
			list_add(mutexes_kernel, mutex);
			pthread_mutex_unlock(&mutex_mutexes_kernel);

			free(nombre_mutex);
			eliminar_paquete(contenido);

			// syscall atendida: el mismo proceso vuelve a la CPU (la CPU lo liberó al hacer la syscall)
			enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
			break;
		}

		case ENVIO_MUTEX_LOCK: // MUTEX_LOCK (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_LOCK");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int));
			char *nombre_mutex = malloc(len1 + 1);
			buffer_read(nombre_mutex, contenido->buffer, len1 + 1); // el '\0' viene incluido

			t_mutex_kernel *mutex = ks_buscar_mutex(nombre_mutex);

			if (mutex == NULL)
			{
				log_warning(ks_logger, "El mutex %s no se encontró. El proceso %d en la cpu %d, no puede tomarlo", nombre_mutex, cpu->pcb_ejecutando->pid, cpu->id);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				// syscall no bloqueante (el mutex no existe): el mismo proceso vuelve a la CPU
				enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
				break;
			}

			pthread_mutex_lock(&mutex->mutex);
			if (!mutex->tomado)
			{ // El mutex no está tomado, lo tomo y termino
				mutex->tomado = true;
				mutex->duenio = cpu->pcb_ejecutando;

				pthread_mutex_unlock(&mutex->mutex);

				log_mutex_tomado(cpu->pcb_ejecutando, nombre_mutex);

				free(nombre_mutex);
				eliminar_paquete(contenido);

				// lo tomó sin bloquearse: el mismo proceso vuelve a la CPU
				enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
				break;
			}

			// El mutex estaba tomado. HERENCIA DE PRIORIDAD (menor número = mayor
			// prioridad): subo al dueño SOLO si el que se bloquea es MÁS prioritario.
			t_pcb *duenio = mutex->duenio;
			bool hubo_herencia = false;
			int prioridad_vieja_duenio = 0;
			if (cpu->pcb_ejecutando->prioridad < duenio->prioridad)
			{
				prioridad_vieja_duenio = duenio->prioridad;
				duenio->prioridad = cpu->pcb_ejecutando->prioridad;
				hubo_herencia = true;
			}

			queue_push(mutex->bloqueados, cpu->pcb_ejecutando);
			pthread_mutex_unlock(&mutex->mutex);

			if (hubo_herencia)
			{
				log_cambio_prioridad(duenio, prioridad_vieja_duenio);
				// Si el dueño estaba esperando en READY, hay que moverlo a la cola
				// de su nueva prioridad; si no, nunca se planificaría antes.
				ks_reubicar_ready_por_cambio_prioridad(duenio, prioridad_vieja_duenio, duenio->prioridad);
			}

			ks_pasar_a_block(cpu->pcb_ejecutando);

			free(nombre_mutex);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MUTEX_UNLOCK: // MUTEX_UNLOCK (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_UNLOCK");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int)); // LEEMOS CUANTO MIDE (strlen sin '\0')
			char *nombre_mutex = malloc(len1 + 1);
			buffer_read(nombre_mutex, contenido->buffer, len1 + 1); // el '\0' viene incluido

			t_mutex_kernel *mutex = ks_buscar_mutex(nombre_mutex);

			if (mutex == NULL)
			{
				log_warning(ks_logger, "El mutex %s no se encontró. El proceso %d en la cpu %d, no puede tomarlo", nombre_mutex, cpu->pcb_ejecutando->pid, cpu->id);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				// syscall no bloqueante (el mutex no existe): el mismo proceso vuelve a la CPU
				enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
				break;
			}

			pthread_mutex_lock(&mutex->mutex);
			if (mutex->duenio != cpu->pcb_ejecutando)
			{
				// Un no-dueño no libera el mutex ni toca la prioridad del dueño
				// (además, duenio podría ser NULL si el mutex está libre).
				pthread_mutex_unlock(&mutex->mutex);
				log_warning(ks_logger, "El proceso %d intentó liberar el mutex %s que no le pertenece", cpu->pcb_ejecutando->pid, nombre_mutex);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				// UNLOCK no bloquea: el proceso vuelve a la CPU
				enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
				break;
			}

			// El dueño libera: si venía con prioridad heredada, la restauro a la
			// original. Está EXEC (ejecutó el UNLOCK), así que alcanza con el campo.
			if (cpu->pcb_ejecutando->prioridad != cpu->pcb_ejecutando->prioridad_original)
			{
				int prioridad_anterior = cpu->pcb_ejecutando->prioridad; // la heredada, antes de restaurar
				cpu->pcb_ejecutando->prioridad = cpu->pcb_ejecutando->prioridad_original;
				log_cambio_prioridad(cpu->pcb_ejecutando, prioridad_anterior);
			}

			log_mutex_liberado(mutex->duenio, nombre_mutex);

			if (queue_is_empty(mutex->bloqueados))
			{
				mutex->tomado = false;
				mutex->duenio = NULL;

				pthread_mutex_unlock(&mutex->mutex);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				// UNLOCK no bloquea: el proceso vuelve a la CPU
				enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
				break;
			}

			t_pcb *desbloqueado = queue_pop(mutex->bloqueados);
			mutex->duenio = desbloqueado;

			log_mutex_tomado(desbloqueado, nombre_mutex);
			pthread_mutex_unlock(&mutex->mutex);

			// BLOCK -> READY (el que estaba esperando el mutex)
			// desbloqueado->estado = READY;
			// pthread_mutex_lock(&array_queues[ks_indice_cola(desbloqueado->prioridad)].mutex_ready);
			// queue_push(array_queues[ks_indice_cola(desbloqueado->prioridad)].queue_ready, desbloqueado);
			// pthread_mutex_unlock(&array_queues[ks_indice_cola(desbloqueado->prioridad)].mutex_ready);
			// sem_post(&sem_ready); // sin esto el corto plazo no levanta al desbloqueado
			ks_desbloquear_proceso(desbloqueado);

			// el que hizo UNLOCK no se bloquea: vuelve a la CPU
			enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);

			free(nombre_mutex);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MEM_ALLOC: // MEM_ALLOC (Id del Segmento, Tamaño, PID)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MEM_ALLOC");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			int id_segmento;
			buffer_read(&id_segmento, contenido->buffer, sizeof(int));

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			enviar_mem_alloc(id_segmento, tamanio, pid);

			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MEM_FREE: // MEM_FREE (Id del Segmento, PID)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MEM_FREE");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			int id_segmento;
			buffer_read(&id_segmento, contenido->buffer, sizeof(int));

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			enviar_mem_free(id_segmento, pid);

			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_SLEEP: // SLEEP (Tiempo)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "SLEEP");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			// CPU manda [pid][tiempo] (execute_SLEEP), en ese orden — como STDIN/STDOUT.
			int pid_sleep;
			buffer_read(&pid_sleep, contenido->buffer, sizeof(int));
			int tiempo;
			buffer_read(&tiempo, contenido->buffer, sizeof(int));

			// El proceso hace IO: se bloquea SIEMPRE. Si no hay IO SLEEP libre, el
			// pedido queda encolado y se despacha cuando una se libere.
			ks_pasar_a_block(cpu->pcb_ejecutando);
			ks_pedir_io_sleep(pid_sleep, tiempo);

			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_STDOUT: // STDOUT (Registro Dirección Lógica, Registro Tamaño, pid)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "STDOUT");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			// CPU manda [pid:int][dir_logica:uint32][tamanio:uint32] (execute_STDOUT)
			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			uint32_t dir_logica;
			buffer_read(&dir_logica, contenido->buffer, sizeof(uint32_t));

			uint32_t tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(uint32_t));

			ks_pasar_a_block(cpu->pcb_ejecutando);

			// la IO STDOUT se reserva recién cuando KM devuelve los datos (STDOUT_RESTUESTA_LECTURA)
			enviar_solicitud_stdout_km(pid, dir_logica, (int)tamanio);

			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_STDIN: // STDIN (Registro Dirección Lógica, Registro Tamaño (DIRECCION LOGICA DONDE SE GUARDARÁ LO LEIDO))
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "STDIN");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			// CPU manda [pid:int][dir_logica:uint32][tamanio:uint32] (execute_STDIN)
			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			uint32_t dir_logica;
			buffer_read(&dir_logica, contenido->buffer, sizeof(uint32_t));

			uint32_t tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(uint32_t));

			// El proceso hace IO: se bloquea SIEMPRE. Si no hay IO STDIN libre, el
			// pedido queda encolado (con su dir_logica) y se despacha al liberarse una.
			// ks_pedir_io_stdin guarda pid/dir/tamanio en la IO asignada, porque
			// OK_STDIN trae solo los datos leídos (sin pid ni dir).
			ks_pasar_a_block(cpu->pcb_ejecutando);
			ks_pedir_io_stdin(pid, dir_logica, (int)tamanio);

			eliminar_paquete(contenido);

			break;
		}

		case ENVIO_INIT_PROC: // INIT_PROC (Archivo de instrucciones, Prioridad)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "INIT_PROC");
			t_paquete *contenido = recibir_paquete(cpu->fd);

			// CPU manda [pid_creador][tam_path][path con '\0'][prioridad] (execute_INIT_PROC)
			int pid_creador;
			buffer_read(&pid_creador, contenido->buffer, sizeof(int));

			int tam_path;
			buffer_read(&tam_path, contenido->buffer, sizeof(int));

			char *archivo_instrucciones = malloc(tam_path + 1);
			buffer_read(archivo_instrucciones, contenido->buffer, tam_path + 1); // el '\0' viene incluido

			int prioridad;
			buffer_read(&prioridad, contenido->buffer, sizeof(int));

			int nuevo_pid = asignar_pid();
			registrar_creacion_pendiente(nuevo_pid, archivo_instrucciones, prioridad);
			enviar_solicitud_creacion_proceso_km(nuevo_pid, archivo_instrucciones, prioridad);

			free(archivo_instrucciones);
			eliminar_paquete(contenido);

			// INIT_PROC no bloquea al creador: vuelve a la CPU
			enviar_alojar_proceso_cpu(cpu->pcb_ejecutando);
			break;
		}

		case ENVIO_EXIT:
		{
			// La CPU manda ENVIO_EXIT [pid]: hay que drenar el paquete aunque el pid no
			// se use (KS confía en cpu->pcb_ejecutando->pid). Sin esto los 8 bytes del
			// payload [size][pid] quedaban en el socket y se leían como opcodes basura
			// (4 y 0) en la siguiente vuelta del loop.
			t_paquete *contenido = recibir_paquete(cpu->fd);
			eliminar_paquete(contenido);
			log_syscall_recibida(cpu->pcb_ejecutando, "EXIT");
			finalizar_proceso(cpu->pcb_ejecutando->pid, "EXIT"); // finalizar_proceso ya loguea el fin
			cpu->pcb_ejecutando = NULL;
			sem_post(&sem_cpu_libre);
			break;
		}

		case DESALOJO_PROCESO: // OK_INTERRUPCION (PID, MOTIVO DE INTERRUPCION)
		{
			t_paquete *contenido = recibir_paquete(cpu->fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			// CPU manda el motivo como int (los valores 3004/3005 no entran en 1 byte).
			int motivo_interrupcion = 0;
			buffer_read(&motivo_interrupcion, contenido->buffer, sizeof(int));

			if (motivo_interrupcion == AVISO_DE_INTERRUPCION_DESALOJO)
			{
				log_debug(ks_logger, "Se desalojo el proceso %d, por motivo de AVISO_DE_INTERRUPCION_DESALOJO", pid);

				t_pcb *pcb = buscar_pcb_por_pid(pid);

				// El mismo aviso sirve para compactación y para desalojo por prioridad.
				// Los distingo por el flag: compactación → ready prioritario (spec);
				// desalojo por prioridad → ready normal de su prioridad.
				pthread_mutex_lock(&mutex_compactacion);
				bool en_compactacion = compactacion_en_progreso;
				pthread_mutex_unlock(&mutex_compactacion);

				ks_termina_de_desalojar_un_pcb_general(pcb, en_compactacion);
				cpu->pcb_ejecutando = NULL;
				sem_post(&sem_cpu_libre);

				// Si era por compactación, contabilizo la CPU desalojada; al llegar
				// a 0 se confirma a KM.
				if (en_compactacion)
				{
					ks_confirmar_desalojo_compactacion_de_una_cpu();
				}
			}
			else if (motivo_interrupcion == AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT)
			{
				log_debug(ks_logger, "Se desalojo el proceso %d, por motivo de AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT", pid);

				// SEG_FAULT: el proceso se FINALIZA (spec), no vuelve a READY. La CPU
				// ya NO se desconecta (fix C6 de CPU), así que la liberamos para el
				// próximo proceso.
				finalizar_proceso(pid, "SEG_FAULT");
				cpu->pcb_ejecutando = NULL;
				sem_post(&sem_cpu_libre);
			}
			else
			{
				log_error(ks_logger, "Se quiere desalojar el proceso %d, por motivo desconocido (%d).", pid, motivo_interrupcion);
			}

			eliminar_paquete(contenido);
			break;
		}

			// Viejo ------------------------------------------

			// case OK_INTERRUPCION: // OK_INTERRUPCION (PID, MOTIVO DE INTERRUPCION)
			// {
			// 	t_paquete* contenido = recibir_paquete(cpu->fd);

			// 	int pid;
			// 	buffer_read(&pid, contenido->buffer, sizeof(int));

			// 	char* motivo_interrupcion = NULL;
			// 	buffer_read(&motivo_interrupcion, contenido->buffer, sizeof(char*));

			// 	free(motivo_interrupcion);
			// 	eliminar_paquete(contenido);
			// 	break;
			// }

			// case OK_DESALOJO_COMPACTACION_MEMORIA: {
			//     ks_termina_de_desalojar_por_compactacion();
			// 	break;
			// }

			// case OK_DESALOJO: // OK_DESALOJO (PID)
			// {
			// 	t_paquete* contenido = recibir_paquete(cpu->fd);

			// 	int pid;
			// 	buffer_read(&pid, contenido->buffer, sizeof(int));

			// 	eliminar_paquete(contenido);
			// 	break;
			// }

			// case OK_ALOJAR: // OK_ALOJAR (PID)
			// {
			// 	t_paquete* contenido = recibir_paquete(cpu->fd);

			// 	int pid;
			// 	buffer_read(&pid, contenido->buffer, sizeof(int));

			// 	eliminar_paquete(contenido);
			// 	break;
			// }

		case -1:
		{
			log_warning(ks_logger, "[CPU %d] se desconecto cpu", cpu->id);
			ejecutando = 0;
			// HACER eliminar cpu de la lista de cpus, y manejar al proc que se esta ejecutando en esa cpu (pasarlo a ready o a block segun corresponda)
			finalizar_cpu(cpu);
			break;
		}
		default:
		{
			log_warning(ks_logger, "[CPU %d] operacion desconocida: %d", cpu->id, op_code);
			break;
		}
		}
	}
	close(cpu->fd);
	return NULL;
}

t_mutex_kernel *ks_buscar_mutex(char *nombre)
{
	bool _es_mutex(void *ptr)
	{
		t_mutex_kernel *mutex = ptr;
		return strcmp(mutex->nombre, nombre) == 0;
	}

	pthread_mutex_lock(&mutex_mutexes_kernel);
	t_mutex_kernel *encontrado = list_find(mutexes_kernel, _es_mutex);
	pthread_mutex_unlock(&mutex_mutexes_kernel);

	return encontrado;
}
