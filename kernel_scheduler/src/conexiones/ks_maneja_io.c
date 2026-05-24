#include "ks_maneja_io.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../main/ks_main.h"
#include "../planificadores/ks_planificador_corto_plazo.h"

void* ks_maneja_io(void* arg) {
	int fd = *(int*)arg;
	free(arg);

	log_debug(ks_logger, "Entra en ks_maneja_io (fd = %d)", fd);
	log_conexion_io((t_io*)arg);

	int ejecutando = 1;

	while (ejecutando) {
		int op_code = recibir_operacion(fd);
		switch (op_code) {
		case OK_SLEEP: // OK_SLEEP (TIEMPO EN MS) (PID)
		{
			t_paquete* contenido = recibir_paquete(fd);

			int tiempo_en_ms;
			buffer_read(&tiempo_en_ms, contenido->buffer, sizeof(int));

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
				log_desbloqueo_sleep(pcb, tiempo_en_ms);
			}

			eliminar_paquete(contenido);
			break;
		}

		case OK_STDIN: // OK_STDIN(bytes)
		{
			t_paquete* contenido = recibir_paquete(fd);

			int bytes;
			buffer_read(&bytes, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
			}

			eliminar_paquete(contenido);
			break;
		}

		case OK_STDOUT_RESPUESTA_BYTES: // OK_STDOUT_RESPUESTA_BYTES (PID)
		{
			t_paquete* contenido = recibir_paquete(fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_pcb* pcb = buscar_pcb_por_pid(pid);

			if (pcb != NULL) {
				ks_desbloquear_proceso(pcb);
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
	close(fd);
	return NULL;
}

void ks_desbloquear_proceso(t_pcb* pcb) {

	if (pcb->estado == BLOCK) {
		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_block);

		bool _es_pcb(void* ptr) {
			t_pcb* aux = ptr;
			return aux->pid == pcb->pid;
		}

		list_remove_by_condition(array_queues[pcb->prioridad].list_block, _es_pcb);

		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_block);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready);
		queue_push(array_queues[pcb->prioridad].queue_ready, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready);

		log_cambio_estado(pcb, (estados BLOCK), (estados READY));

		// sem_post(&sem_ready);
		return;
	}

	if (pcb->estado == SUSP_BLOCK) {

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_susp_block);

		bool _es_pcb(void* ptr) {
			t_pcb* aux = ptr;
			return aux->pid == pcb->pid;
		}

		list_remove_by_condition(array_queues[pcb->prioridad].list_susp_block, _es_pcb);

		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_susp_block);

		pcb->estado = SUSP_READY;
		pcb->tiempo_inicio_suspendido = time(NULL);

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_susp_ready);
		queue_push(array_queues[pcb->prioridad].queue_susp_ready, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_susp_ready);

		log_cambio_estado(pcb, (estados SUSP_BLOCK), (estados SUSP_READY));

		sem_post(&sem_memoria_disponible);
	}
}

void ks_io_sleep(t_pcb* pcb, int tiempo_ms) {

	ks_pasar_a_block(pcb);

	enviar_sleep_io(pcb->io_fd, tiempo_ms, pcb->pid);
}

void ks_io_stdin(t_pcb* pcb, int tamanio, int direccion_logica) {

	ks_pasar_a_block(pcb);

	enviar_stdin_io(pcb->io_fd, tamanio);

	// TODO: despues cuando vuelva:
	// enviar a KM
}

void ks_io_stdout(t_pcb* pcb, int tamanio, int direccion_logica) {

	ks_pasar_a_block(pcb);

	// TODO: pedir bytes a KM
	// luego enviarselos a IO

	enviar_stdout_io(pcb->io_fd, tamanio);
}