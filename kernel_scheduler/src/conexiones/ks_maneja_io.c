#include "ks_maneja_io.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../main/ks_main.h"
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

			eliminar_paquete(contenido);
			break;
		}

		case OK_STDIN: // OK_STDIN(pid)
		{
			t_paquete* contenido = recibir_paquete(io->fd);

			int bytes;
			buffer_read(&bytes, contenido->buffer, sizeof(int));

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
				log_debug(ks_logger, "Proceso %d desbloqueado del STDIN", pid);
				log_fin_io(pcb);
			}

			// TODO(KS): flujo STDIN incompleto — OK_STDIN no trae los datos leídos ni KS
			// recuerda la dirección lógica del pedido. Cuando esté eso, llamar:
			// enviar_bytes_stdin(pid, dir_logica, tamanio, datos);  (STDIN_SOLICITUD_ESCRITURA a KM)

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

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_block);
		list_remove_by_condition(array_queues[pcb->prioridad].list_block, _es_pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_block);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready);
		queue_push(array_queues[pcb->prioridad].queue_ready, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready);

		log_cambio_estado(pcb, BLOCK, READY);

		// sem_post(&sem_ready);
		return;
	}

	if (pcb->estado == SUSP_BLOCK) {
		bool _es_pcb(void* ptr) {
			t_pcb* aux = ptr;
			return aux->pid == pcb->pid;
		}
		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_susp_block);
		list_remove_by_condition(array_queues[pcb->prioridad].list_susp_block, _es_pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_susp_block);

		pcb->estado = SUSP_READY;
		pcb->tiempo_inicio_suspendido = time(NULL);

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_susp_ready);
		list_add(array_queues[pcb->prioridad].list_susp_ready, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_susp_ready);

		log_cambio_estado(pcb, SUSP_BLOCK, SUSP_READY);

		sem_post(&sem_memoria_disponible);
	}
}
