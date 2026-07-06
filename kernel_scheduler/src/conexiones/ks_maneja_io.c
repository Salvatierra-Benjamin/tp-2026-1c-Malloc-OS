#include "ks_maneja_io.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../main/ks_main.h"
#include "../mensajes/ks_mensajes.h"
#include "../planificadores/ks_planificador_corto.h"

void* ks_maneja_io(void* arg) {
	// main pasa un t_io*; NO liberarlo acá, vive en las listas ios_*
	t_io* io = (t_io*)arg;

	log_debug(ks_logger, "Entra en ks_maneja_io (fd = %d)", io->fd);

	int ejecutando = 1;

	while (ejecutando) {
		int op_code = recibir_operacion(io->fd);
		switch (op_code) {
		case OK_SLEEP: // OK_SLEEP  (PID)
		{
			t_paquete* contenido = recibir_paquete(io->fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
				log_debug(ks_logger, "Proceso %d desbloqueado del SLEEP", pid);
				log_fin_io(pcb);
			}

			// La IO terminó: se libera (o toma el próximo SLEEP encolado).
			ks_liberar_io_sleep(io);

			eliminar_paquete(contenido);
			break;
		}

		case OK_STDIN: // IO -> KS: payload = [datos] (tamaño = lo pedido). pid/dir los tengo en la IO
		{
			t_paquete* contenido = recibir_paquete(io->fd);

			int pid = io->pid_pendiente;
			uint32_t dir_logica = io->stdin_dir_logica;
			int tamanio = io->stdin_tamanio;

			// Leo lo que IO capturó del teclado y se lo mando a KM para que lo
			// escriba en la memoria del proceso (dir_logica).
			void* datos = malloc(tamanio);
			buffer_read(datos, contenido->buffer, tamanio);

			// OJO: NO se desbloquea acá. El proceso vuelve a READY recién cuando KM
			// confirma la escritura con OK_STDIN_DATOS_ESCRITOS (ks_maneja_km).
			enviar_bytes_stdin(pid, dir_logica, tamanio, datos);

			free(datos);

			// La IO ya leyó (su trabajo terminó, aunque el proceso se desbloquee
			// recién con OK_STDIN_DATOS_ESCRITOS): se libera o toma el próximo STDIN.
			ks_liberar_io_stdin(io);

			eliminar_paquete(contenido);
			break;
		}

		case OK_STDOUT: // OK_STDOUT_RESPUESTA_BYTES (PID)
		{
			t_paquete* contenido = recibir_paquete(io->fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
				log_debug(ks_logger, "Proceso %d desbloqueado del STDOUT", pid);
				log_fin_io(pcb);
			}

			// La IO terminó de imprimir: se libera (o toma el próximo STDOUT encolado).
			ks_liberar_io_stdout(io);

			eliminar_paquete(contenido);
			break;
		}

		case -1: {
			log_warning(ks_logger, "[IO] se desconecto io");
			ejecutando = 0;
			break;
		}
		default: {
			log_warning(ks_logger, "[IO] operacion desconocida: %d", op_code);
			break;
		}
		}
	}
	close(io->fd);
	return NULL;
}

void ks_desbloquear_proceso(t_pcb* pcb) {

	if (pcb->estado == BLOCK) {
		bool _es_pcb(void* ptr) {
			t_pcb* aux = ptr;
			return aux->pid == pcb->pid;
		}

		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_block);
		list_remove_by_condition(array_queues[ks_indice_cola(pcb->prioridad)].list_block, _es_pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_block);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready);
		queue_push(array_queues[ks_indice_cola(pcb->prioridad)].queue_ready, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_ready);

		log_cambio_estado(pcb, BLOCK, READY);

		sem_post(&sem_ready); // sin esto el corto plazo no vuelve a planificar al proceso
		return;
	}

	if (pcb->estado == SUSP_BLOCK) {
		bool _es_pcb(void* ptr) {
			t_pcb* aux = ptr;
			return aux->pid == pcb->pid;
		}
		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_susp_block);
		list_remove_by_condition(array_queues[ks_indice_cola(pcb->prioridad)].list_susp_block, _es_pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_susp_block);

		pcb->estado = SUSP_READY;
		// NO se re-setea tiempo_inicio_suspendido: se mantiene el instante en que el
		// proceso se SUSPENDIÓ (BLOCK→SUSP_BLOCK) para ordenar la des-suspensión por
		// "mayor tiempo suspendido" (spec). Pisarlo acá mediría solo el tiempo en SUSP_READY.

		pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_susp_ready);
		list_add(array_queues[ks_indice_cola(pcb->prioridad)].list_susp_ready, pcb);
		pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_susp_ready);

		log_cambio_estado(pcb, SUSP_BLOCK, SUSP_READY);

		sem_post(&sem_memoria_disponible);
	}
}
