#include "ks_maneja_cpu.h"

#include "../logs/ks_logs.h"
#include "../main/ks_main.h"

void* ks_maneja_cpu(void* arg) {
	t_cpu* cpu = (t_cpu*)arg;
	// free(arg);

	log_debug(ks_logger, "Entra en ks_maneja_cpu (fd = %d)", cpu->fd);
	log_conexion_cpu(cpu->id);

	int ejecutando = 1;

	while (ejecutando) {
		int op_code = recibir_operacion(cpu->fd);
		switch (op_code) {
		case ENVIO_MUTEX_CREATE: // MUTEX_CREATE (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_CREATE");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int));	 // LEEMOS CUANTO MIDE
			char* nombre_mutex = malloc(len1);					 // PEDIMOS EL ESPACIO JUSTO
			buffer_read(&nombre_mutex, contenido->buffer, len1); // COPIAMOS EL CONTENIDO

			t_mutex_kernel* mutex = malloc(sizeof(t_mutex_kernel));

			strcpy(mutex->nombre, nombre_mutex);
			mutex->tomado = false;
			mutex->duenio = NULL;
			mutex->bloqueados = queue_create();
			pthread_mutex_init(&mutex->mutex, NULL);

			pthread_mutex_lock(&mutex_mutexes_kernel);
			list_add(mutexes_kernel, mutex);
			pthread_mutex_unlock(&mutex_mutexes_kernel);

			free(nombre_mutex);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MUTEX_LOCK: // MUTEX_LOCK (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_LOCK");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int));	 // LEEMOS CUANTO MIDE
			char* nombre_mutex = malloc(len1);					 // PEDIMOS EL ESPACIO JUSTO
			buffer_read(&nombre_mutex, contenido->buffer, len1); // COPIAMOS EL CONTENIDO

			t_mutex_kernel* mutex = ks_buscar_mutex(nombre_mutex);

			if (mutex == NULL) {
				log_warning(ks_logger, "El mutex %s no se encontró. El proceso %d en la cpu %d, no puede tomarlo", nombre_mutex, cpu->pcb_ejecutando->pid, cpu->id);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				break;
			}

			pthread_mutex_lock(&mutex->mutex);
			if (!mutex->tomado) { // El mutex no está tomado, lo tomo y termino
				mutex->tomado = true;
				mutex->duenio = cpu->pcb_ejecutando;

				pthread_mutex_unlock(&mutex->mutex);

				log_mutex_tomado(cpu->pcb_ejecutando, nombre_mutex);

				break;
			}

			// El mutex estaba tomado, reviso si hay herencia de prioridad
			// HERENCIA PRIORIDAD
			if (cpu->pcb_ejecutando->prioridad > mutex->duenio->prioridad) {
				mutex->duenio->prioridad = cpu->pcb_ejecutando->prioridad;
				log_cambio_prioridad(mutex->duenio);
			}

			queue_push(mutex->bloqueados, cpu->pcb_ejecutando);
			pthread_mutex_unlock(&mutex->mutex);

			ks_pasar_a_block(cpu->pcb_ejecutando);

			free(nombre_mutex);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MUTEX_UNLOCK: // MUTEX_UNLOCK (Nombre Mutex)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MUTEX_UNLOCK");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int));	 // LEEMOS CUANTO MIDE
			char* nombre_mutex = malloc(len1);					 // PEDIMOS EL ESPACIO JUSTO
			buffer_read(&nombre_mutex, contenido->buffer, len1); // COPIAMOS EL CONTENIDO

			t_mutex_kernel* mutex = ks_buscar_mutex(nombre_mutex);

			if (mutex == NULL) {
				log_warning(ks_logger, "El mutex %s no se encontró. El proceso %d en la cpu %d, no puede tomarlo", nombre_mutex, cpu->pcb_ejecutando->pid, cpu->id);
				free(nombre_mutex);
				eliminar_paquete(contenido);
				break;
			}

			pthread_mutex_lock(&mutex->mutex);
			if (mutex->duenio != cpu->pcb_ejecutando) {
				mutex->duenio->prioridad = mutex->duenio->prioridad_original;
				log_cambio_prioridad(mutex->duenio);
				pthread_mutex_unlock(&mutex->mutex);
				break;
			}

			log_mutex_liberado(mutex->duenio, nombre_mutex);

			if (queue_is_empty(mutex->bloqueados)) {
				mutex->tomado = false;
				mutex->duenio = NULL;

				pthread_mutex_unlock(&mutex->mutex);
				break;
			}

			t_pcb* desbloqueado = queue_pop(mutex->bloqueados);
			mutex->duenio = desbloqueado;

			log_mutex_tomado(desbloqueado, nombre_mutex);
			pthread_mutex_unlock(&mutex->mutex);

			// BLOCK -> READY
			desbloqueado->estado = READY;
			pthread_mutex_lock(&array_queues[desbloqueado->prioridad].mutex_ready);
			queue_push(array_queues[desbloqueado->prioridad].queue_ready, desbloqueado);
			pthread_mutex_unlock(&array_queues[desbloqueado->prioridad].mutex_ready);

			free(nombre_mutex);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_MEM_ALLOC: // MEM_ALLOC (Id del Segmento, Tamaño, PID)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "MEM_ALLOC");
			t_paquete* contenido = recibir_paquete(cpu->fd);

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
			t_paquete* contenido = recibir_paquete(cpu->fd);

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
			t_paquete* contenido = recibir_paquete(fd);

			int tiempo;
			buffer_read(&tiempo, contenido->buffer, sizeof(int));

			t_io* io = io_libre("SLEEP");

			if (io == NULL) {
				log_warning(ks_logger, "No hay IOs libres para el proceso %d en la cpu %d, reintente el SLEEP luego", cpu->pcb_ejecutando, cpu->id_cpu);
				return; // ? Lo mantengo???
			}

			cpu->pcb_ejecutando->io_fd = io->fd;

			ks_pasar_a_block(cpu->pcb_ejecutando);

			enviar_sleep_io(io->fd, tiempo, cpu->pcb_ejecutando->pid);

			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_STDOUT: // STDOUT (Registro Dirección Lógica, Registro Tamaño, pid)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "STDOUT");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			char* dir_fisica = NULL;
			buffer_read(&dir_fisica, contenido->buffer, sizeof(char*));

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			t_io* io = io_libre("STDOUT");

			if (io == NULL) {
				log_warning(ks_logger, "No hay IOs libres para el proceso %d en la cpu %d, reintente el STDOUT luego", cpu->pcb_ejecutando, cpu->id_cpu);
				return; // ? Lo mantengo??? O pongo break???
			}

			cpu->pcb_ejecutando->io_fd = io->fd;

			ks_pasar_a_block(cpu->pcb_ejecutando);

			enviar_bytes_stdin(pid, len, dir_fisica, tamanio);

			free(dir_fisica);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_STDIN: // STDIN (Registro Dirección Lógica, Registro Tamaño (DIRECCION LOGICA DONDE SE GUARDARÁ LO LEIDO))
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "STDIN");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			char* dir_logica = NULL;
			buffer_read(&dir_logica, contenido->buffer, sizeof(char*));

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			t_io* io = io_libre("STDIN");

			if (io == NULL) {
				log_warning(ks_logger, "No hay IOs libres para el proceso %d en la cpu %d, reintente el STDIN luego", cpu->pcb_ejecutando, cpu->id_cpu);
				return; // ? Lo mantengo???
			}

			cpu->pcb_ejecutando->io_fd = io->fd;

			ks_pasar_a_block(cpu->pcb_ejecutando);

			enviar_stdin_io(io->fd, dir_logica, tamanio);

			free(dir_logica);
			eliminar_paquete(contenido);

			break;
		}

		case ENVIO_INIT_PROC: // INIT_PROC (Archivo de instrucciones, Prioridad)
		{
			log_syscall_recibida(cpu->pcb_ejecutando, "INIT_PROC");
			t_paquete* contenido = recibir_paquete(cpu->fd);

			char* archivo_insrucciones = NULL;
			buffer_read(&archivo_insrucciones, contenido->buffer, sizeof(char*));

			int prioridad;
			buffer_read(&prioridad, contenido->buffer, sizeof(int));

			enviar_solicitud_creacion_proceso_km(archivo_insrucciones, prioridad);

			free(archivo_insrucciones);
			eliminar_paquete(contenido);
			break;
		}

		case ENVIO_EXIT: {
			log_syscall_recibida(cpu->pcb_ejecutando, "EXIT");
			log_fin_proceso(cpu->pcb_ejecutando, "EXIT");
			finalizar_proceso(cpu->pcb_ejecutando);
			break;
		}

		case DESALOJO_PROCESO: // OK_INTERRUPCION (PID, MOTIVO DE INTERRUPCION)
		{
			t_paquete* contenido = recibir_paquete(cpu->fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			uint8_t motivo_interrupcion = 0; //
			buffer_read(&motivo_interrupcion, contenido->buffer, sizeof(uint8_t));

			if (motivo_interrupcion == AVISO_DE_INTERRUPCION_DESALOJO) {
				log_debug(ks_logger, "Se desalojo el proceso %d, por motivo de AVISO_DE_INTERRUPCION_DESALOJO", pid);

				t_pcb* pcb = ks_buscar_pcb_por_pid(pid);
				ks_termina_de_desalojar_general(pcb);
			} else if (motivo_interrupcion == AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT) {
				log_debug(ks_logger, "Se desalojo el proceso %d, por motivo de AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT", pid);

				t_pcb* pcb = ks_buscar_pcb_por_pid(pid);
				ks_termina_de_desalojar_un_pcb_seg_fault(pcb);

				// ? Se tiene que desconectar la cpu, y eso lo manejo en el swicth cuando se desconecta (case -1).
			} else {
				log_error(ks_logger, "Se quiere desalojar el proceso %d, por motivo desconocido (%d).", pid, motivo_interrupcion);
			}

			eliminar_paquete(contenido);
			break;
		}

			// ! Viejo ------------------------------------------

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

		case -1: {
			log_warning(logger, "[CPU %d] se desconecto cpu", cpu->id);
			ejecutando = 0;
			// TODO: eliminar cpu de la lista de cpus, y manejar al proc que se esta ejecutando en esa cpu (pasarlo a ready o a block segun corresponda)
			finalizar_cpu(cpu);
			break;
		}
		default: {
			log_warning(logger, "[CPU %d] operacion desconocida: %d", cpu->id, op_code);
			break;
		}
		}
	}
	close(fd);
	return NULL;
}

t_mutex_kernel* ks_buscar_mutex(char* nombre) {
	bool _es_mutex(void* ptr) {
		t_mutex_kernel* mutex = ptr;
		return strcmp(mutex->nombre, nombre) == 0;
	}

	pthread_mutex_lock(&mutex_mutexes_kernel);
	t_mutex_kernel* encontrado = list_find(mutexes_kernel, _es_mutex);
	pthread_mutex_unlock(&mutex_mutexes_kernel);

	return encontrado;
}
