#include "ks_auxiliares.h"

#include "../main/ks_main.h"
#include "../mensajes/ks_mensajes.h"
#include "../planificadores/ks_planificador_mediano.h" // ks_timer_suspension

void liberar_io(void* io_original) {
	t_io* io = (t_io*)io_original;
	free(io); // Liberar la memoria de la IO
}

void liberar_cpu(void* cpu_original) {
	t_cpu* cpu = (t_cpu*)cpu_original;
	free(cpu->pcb_ejecutando);
	free(cpu); // Liberar la memoria del CPU
}

void liberar_proceso(void* proc) {
	t_pcb* proceso = (t_pcb*)proc;
	free(proceso->path); // Liberar la memoria del path
	free(proceso);		 // Liberar la memoria del PCB
}

void liberar_proceso_bsod(void* proc) {
	t_pcb* proceso = (t_pcb*)proc;
	log_fin_proceso(proceso, "Blue Screen of Death (BSOD)");
	free(proceso->path); // Liberar la memoria del path
	free(proceso);		 // Liberar la memoria del PCB
}

void matar_procesos_ready() {
	for (int i = 0; i < cantidad_algoritmos; i++) {
		queue_destroy_and_destroy_elements(array_queues[i].queue_ready_prioritario, liberar_proceso_bsod);
		queue_destroy_and_destroy_elements(array_queues[i].queue_ready, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_block, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_susp_block, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_susp_ready, liberar_proceso_bsod);
	}
}

void liberar_listas_matando_procesos_bsod() {
	pthread_mutex_lock(&mutex_cpus);
	list_destroy_and_destroy_elements(cpus, liberar_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	pthread_mutex_lock(&mutex_ios_sleep);
	list_destroy_and_destroy_elements(ios_sleep, liberar_io);
	pthread_mutex_unlock(&mutex_ios_sleep);

	pthread_mutex_lock(&mutex_ios_stdin);
	list_destroy_and_destroy_elements(ios_stdin, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdin);

	pthread_mutex_lock(&mutex_ios_stdout);
	list_destroy_and_destroy_elements(ios_stdout, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdout);

	pthread_mutex_lock(&mutex_exec);
	list_destroy_and_destroy_elements(procs_exec, liberar_proceso_bsod);
	pthread_mutex_unlock(&mutex_exec);

	list_destroy_and_destroy_elements(procs_exit, liberar_proceso_bsod);
	pthread_mutex_unlock(&mutex_exit);
}

void liberar_listas_matando_procesos() {
	pthread_mutex_lock(&mutex_cpus);
	list_destroy_and_destroy_elements(cpus, liberar_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	pthread_mutex_lock(&mutex_ios_sleep);
	list_destroy_and_destroy_elements(ios_sleep, liberar_io);
	pthread_mutex_unlock(&mutex_ios_sleep);

	pthread_mutex_lock(&mutex_ios_stdin);
	list_destroy_and_destroy_elements(ios_stdin, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdin);

	pthread_mutex_lock(&mutex_ios_stdout);
	list_destroy_and_destroy_elements(ios_stdout, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdout);

	pthread_mutex_lock(&mutex_exec);
	list_destroy_and_destroy_elements(procs_exec, liberar_proceso);
	pthread_mutex_unlock(&mutex_exec);

	list_destroy_and_destroy_elements(procs_exit, liberar_proceso);
	pthread_mutex_unlock(&mutex_exit);
}

void liberar_mutex() {
	pthread_mutex_destroy(&mutex_cpus);
	pthread_mutex_destroy(&mutex_ios_sleep);
	pthread_mutex_destroy(&mutex_ios_stdin);
	pthread_mutex_destroy(&mutex_ios_stdout);
	pthread_mutex_destroy(&mutex_exec);
	// pthread_mutex_destroy(&mutex_block);
	// pthread_mutex_destroy(&mutex_susp_block);
	// pthread_mutex_destroy(&mutex_susp_ready);
	pthread_mutex_destroy(&mutex_exit);
	pthread_mutex_destroy(&mutex_array_queues);

	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_destroy(&array_queues[i].mutex_ready_prioritario);
		pthread_mutex_destroy(&array_queues[i].mutex_ready);
		pthread_mutex_destroy(&array_queues[i].mutex_block);
		pthread_mutex_destroy(&array_queues[i].mutex_susp_block);
		pthread_mutex_destroy(&array_queues[i].mutex_susp_ready);
	}
}

void inicializar_semaforos(void) {
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_cpu_libre, 0, 0);
	sem_init(&sem_memoria_disponible, 0, 0);
	sem_init(&sem_compactacion_finalizada, 0, 0);
}

void destruir_semaforos(void) {
	sem_destroy(&sem_ready);
	sem_destroy(&sem_cpu_libre);
	sem_destroy(&sem_memoria_disponible);
}

bool finalizar_proceso(int pid) {
	t_pcb* pcb = buscar_pcb_por_pid(pid);
	if (pcb == NULL) {
		return false; // No se encontró el proceso con el PID dado
	}
	pcb->estado = EXIT_ESTADO; // Cambia el estado del proceso a "EXIT"
	pthread_mutex_lock(&mutex_exit);
	list_add(procs_exit, pcb); // Agrega el proceso a la lista de procesos en estado EXIT
	pthread_mutex_unlock(&mutex_exit);

	log_fin_proceso(pcb, "EXIT");

	enviar_fin_proceso_km(pid);

	return true;
}

t_pcb* crear_proceso(int pid, char* path, int prioridad) {
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
	if (nuevo_pcb == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nuevo_pcb->pid = pid;
	nuevo_pcb->path = strdup(path);			   // Asegúrate de liberar esta memoria cuando ya no se necesite
	nuevo_pcb->prioridad = prioridad;		   // ! No manejo el caso en el que la prioridadd exceda a la cant de alg, xq dijeron que eso no va a suceder en las pruebas
	nuevo_pcb->prioridad_original = prioridad; // Guardar la prioridad original para herencia de prioridad
	nuevo_pcb->cpu_fd = -1;					   // Inicialmente no asignado a ningún CPU
	nuevo_pcb->io_fd = -1;					   // Inicialmente no asignado a ningún IO

	log_creacion_proceso(nuevo_pcb);

	log_cambio_estado(nuevo_pcb, NEW, READY);

	nuevo_pcb->estado = READY; // Estado inicial del proceso es "READY" (saltea NEW)

	pthread_mutex_lock(&array_queues[prioridad].mutex_ready);
	queue_push(array_queues[prioridad].queue_ready, nuevo_pcb); // Agrega el proceso a la cola correspondiente según su prioridad
	pthread_mutex_unlock(&array_queues[prioridad].mutex_ready);

	return nuevo_pcb;
}

t_cpu* crear_cpu(int id, int fd) {
	t_cpu* nueva_cpu = malloc(sizeof(t_cpu));
	if (nueva_cpu == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nueva_cpu->id = id;
	nueva_cpu->pcb_ejecutando = NULL; // Inicialmente no ejecutando ningún proceso
	nueva_cpu->fd = fd;

	return nueva_cpu;
}

t_io* crear_io(int fd, char* tipo_io) {
	t_io* nuevo_io = malloc(sizeof(t_io));
	if (nuevo_io == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nuevo_io->esta_libre = true;
	nuevo_io->fd = fd;

	if (strcmp(tipo_io, "SLEEP") == 0) {
		pthread_mutex_lock(&mutex_ios_sleep);
		list_add(ios_sleep, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_sleep);
	} else if (strcmp(tipo_io, "STDIN") == 0) {
		pthread_mutex_lock(&mutex_ios_stdin);
		list_add(ios_stdin, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_stdin);
	} else if (strcmp(tipo_io, "STDOUT") == 0) {
		pthread_mutex_lock(&mutex_ios_stdout);
		list_add(ios_stdout, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_stdout);
	}

	log_debug(ks_logger, "IO de tipo %s creada con fd %d", tipo_io, fd);

	return nuevo_io;
}

t_io* io_libre(char* tipo_io) {
	bool _io_esta_libre(void* ptr) {
		t_io* io = (t_io*)ptr;
		return io->esta_libre;
	}

	if (strcmp(tipo_io, "SLEEP") == 0) {
		pthread_mutex_lock(&mutex_ios_sleep);
		t_io* io = list_find(ios_sleep, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_sleep);
		return io;
	} else if (strcmp(tipo_io, "STDIN") == 0) {
		pthread_mutex_lock(&mutex_ios_stdin);
		t_io* io = list_find(ios_stdin, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_stdin);
		return io;
	} else if (strcmp(tipo_io, "STDOUT") == 0) {
		pthread_mutex_lock(&mutex_ios_stdout);
		t_io* io = list_find(ios_stdout, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_stdout);
		return io;
	}
	return NULL;
}

void finalizar_cpu(t_cpu* cpu) {
	if (cpu->pcb_ejecutando != NULL) {
		cpu->pcb_ejecutando->cpu_fd = -1;
		cpu->pcb_ejecutando->io_fd = -1;
		cpu->pcb_ejecutando->estado = READY; // Cambia el estado del proceso a "READY" para que pueda ser planificado nuevamente

		log_cambio_estado(cpu->pcb_ejecutando, EXEC, READY);

		pthread_mutex_lock(&array_queues[cpu->pcb_ejecutando->prioridad].mutex_ready);
		queue_push(array_queues[cpu->pcb_ejecutando->prioridad].queue_ready, cpu->pcb_ejecutando); // Agrega el proceso de vuelta a la cola de ready correspondiente
		pthread_mutex_unlock(&array_queues[cpu->pcb_ejecutando->prioridad].mutex_ready);

		cpu->pcb_ejecutando = NULL;
	}

	pthread_mutex_lock(&mutex_cpus);
	bool _es_cpu(void* ptr) {
		t_cpu* aux = (t_cpu*)ptr;
		return aux->id == cpu->id;
	}
	list_remove_by_condition(cpus, _es_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	free(cpu->pcb_ejecutando);
	free(cpu);
}

void ks_liberar_array_queues(void) {
	if (array_queues) {
		for (int i = 0; i < cantidad_algoritmos; i++) {
			queue_destroy(array_queues[i].queue_ready);
		}

		free(array_queues);
	}
}

t_pcb* buscar_pcb_por_pid(int pid) {
	bool _pcb_tiene_pid(void* ptr) {
		t_pcb* pcb = (t_pcb*)ptr;
		return pcb->pid == pid;
	}

	t_pcb* encontrado = NULL;

	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_lock(&array_queues[i].mutex_ready);
		encontrado = list_find(array_queues[i].queue_ready->elements, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_ready);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_block);
		encontrado = list_find(array_queues[i].list_block, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_block);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_susp_block);
		encontrado = list_find(array_queues[i].list_susp_block, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_block);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_susp_ready);
		encontrado = list_find(array_queues[i].list_susp_ready, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
		if (encontrado != NULL)
			return encontrado;
	}

	pthread_mutex_lock(&mutex_exec);
	encontrado = list_find(procs_exec, _pcb_tiene_pid);
	pthread_mutex_unlock(&mutex_exec);

	if (encontrado != NULL) {
		return encontrado;
	}

	// ptrhread_mutex_lock(&mutex_exit);
	// encontrado = list_find(procs_exit, _pcb_tiene_pid);
	// pthread_mutex_unlock(&mutex_exit);

	// no tiene sentido hacer el if, porque no me ahorro nada y retorna directamente el NULL si no lo encuentra

	return encontrado; // No se encontró el PCB con el PID dado
}

char* estado_a_string(estados estado) {
	switch (estado) {
	case NEW:
		return "NEW";
	case READY:
		return "READY";
	case EXEC:
		return "EXEC";
	case BLOCK:
		return "BLOCK";
	case SUSP_BLOCK:
		return "SUSP_BLOCK";
	case SUSP_READY:
		return "SUSP_READY";
	case EXIT_ESTADO:
		return "EXIT";
	default:
		return "UNKNOWN";
	}
}

void tomar_mutex_y_log(t_pcb* pcb, pthread_mutex_t* mutex, char* nombre_mutex) {
	log_mutex_tomado(pcb, nombre_mutex);
	pthread_mutex_lock(mutex);
}

void liberar_mutex_y_log(t_pcb* pcb, pthread_mutex_t* mutex, char* nombre_mutex) {
	pthread_mutex_unlock(mutex);
	log_mutex_liberado(pcb, nombre_mutex);
}

void ks_pasar_a_block(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_exec);

	bool _es_pcb(void* ptr) {
		t_pcb* aux = ptr;
		return aux->pid == pcb->pid;
	}

	list_remove_by_condition(procs_exec, _es_pcb);

	pthread_mutex_unlock(&mutex_exec);

	pcb->estado = BLOCK;
	pcb->tiempo_inicio_block = time(NULL);

	pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_block);
	list_add(array_queues[pcb->prioridad].list_block, pcb);
	pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_block);

	log_cambio_estado(pcb, EXEC, BLOCK);

	// timer de mediano plazo: si sigue en BLOCK tras SUSPENSION_TIMEOUT pasa a SUSP_BLOCK
	pthread_t hilo_suspension;
	pthread_create(&hilo_suspension, NULL, ks_timer_suspension, pcb);
	pthread_detach(hilo_suspension);

	sem_post(&sem_cpu_libre);
}