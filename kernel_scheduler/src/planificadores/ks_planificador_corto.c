#include "ks_planificador_corto.h"

#include "../main/ks_main.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../mensajes/ks_mensajes.h"

void* ks_planificador_corto_plazo(void* arg) {
	(void)arg;

	while (1) {
		sem_wait(&sem_ready);
		sem_wait(&sem_cpu_libre);

		t_pcb* pcb = NULL;

		pthread_mutex_lock(&mutex_compactacion);
		if (!compactacion_en_progreso) {
			pthread_mutex_unlock(&mutex_compactacion);
			pcb = ks_obtener_siguiente_ready_prioritario(); // Si estoy en FIFO o en RR, no afecta en nada, porque es solo una prioridad, pero si estoy en CMN, me va a asegurar que primero se planifiquen los procesos que fueron desalojados por compactacion
		}
		pthread_mutex_unlock(&mutex_compactacion);

		if (pcb == NULL) {
			pcb = ks_obtener_siguiente_ready();
		}

		if (pcb == NULL) {
			sem_post(&sem_cpu_libre);
			continue;
		}

		switch (ks_config.algoritmo) {
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

t_pcb* ks_obtener_siguiente_ready(void) {
	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_lock(&array_queues[i].mutex_ready);

		if (!queue_is_empty(array_queues[i].queue_ready)) {
			t_pcb* pcb = queue_pop(array_queues[i].queue_ready);
			pthread_mutex_unlock(&array_queues[i].mutex_ready);

			return pcb;
		}

		pthread_mutex_unlock(&array_queues[i].mutex_ready);
	}

	return NULL;
}

t_pcb* ks_obtener_siguiente_ready_prioritario(void) {
	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_lock(&array_queues[i].mutex_ready_prioritario);

		if (!queue_is_empty(array_queues[i].queue_ready_prioritario)) {
			t_pcb* pcb = queue_pop(array_queues[i].queue_ready_prioritario);
			pthread_mutex_unlock(&array_queues[i].mutex_ready_prioritario);

			return pcb;
		}
		pthread_mutex_unlock(&array_queues[i].mutex_ready_prioritario);
	}

	return NULL;
}

void ks_enviar_a_exec(t_pcb* pcb) {
	pcb->estado = EXEC;

	pthread_mutex_lock(&mutex_exec);
	list_add(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	// TODO: buscar cpu libre
	// TODO: enviar dispatch

	log_cambio_estado(pcb, READY, EXEC);
}

void ks_planificar_fifo(t_pcb* pcb) {
	ks_enviar_a_exec(pcb);
}

void ks_planificar_rr(t_pcb* pcb) {
	ks_enviar_a_exec(pcb);

	usleep(ks_config.rr_quantum * 1000); // RR_QUANTUM viene en ms, usleep toma µs

	if (pcb->estado == EXEC) {
		pthread_mutex_lock(&mutex_exec);
		list_remove_element(procs_exec, pcb);
		pthread_mutex_unlock(&mutex_exec);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready);
		queue_push(array_queues[pcb->prioridad].queue_ready, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready);

		log_cambio_estado(pcb, EXEC, READY);

		log_desalojo_quantum(pcb);
		// sem_post(&sem_ready); // ? Hace falta hacerlo??
	}
}

void ks_planificar_cmn(t_pcb* pcb) {

	// pthread_mutex_lock(&mutex_array_queues);
	algoritmos alg = array_queues[pcb->prioridad].alg;
	// pthread_mutex_unlock(&mutex_array_queues);

	switch (alg) {
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

void ks_verificar_desalojo_prioridad(t_pcb* nuevo) {
	if (!ks_config.queue_preemption) {
		return;
	}

	pthread_mutex_lock(&mutex_exec);

	for (int i = 0; i < list_size(procs_exec); i++) {
		t_pcb* exec = list_get(procs_exec, i);

		if (exec->prioridad > nuevo->prioridad) {
			// enviar_interrupcion_cpu(exec->cpu_fd, exec->pid, "Desalojo por prioridad");
			enviar_interrupcion_cpu(exec->cpu_fd, exec->pid);

			log_desalojo_prioridad(exec, nuevo);

			pthread_mutex_unlock(&mutex_exec);
			return;
		}
	}

	pthread_mutex_unlock(&mutex_exec);
}

void ks_empieza_a_desalojar_por_compactacion(void) {
	pthread_mutex_lock(&mutex_exec);

	for (int i = 0; i < list_size(procs_exec); i++) {
		t_pcb* pcb = list_get(procs_exec, i);

		enviar_interrupcion_compactacion_cpu(pcb->cpu_fd, pcb->pid);
	}
	// list_clean(procs_exec);

	pthread_mutex_unlock(&mutex_exec);
}

void ks_termina_de_desalojar_un_pcb_general(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_exec);
	list_remove_element(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	pcb->estado = READY;

	pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready_prioritario);
	queue_push(array_queues[pcb->prioridad].queue_ready_prioritario, pcb);
	pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready_prioritario);

	log_cambio_estado(pcb, EXEC, READY);
	sem_post(&sem_ready);
}

void ks_termina_de_desalojar_un_pcb_seg_fault(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_exec);
	list_remove_element(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	pcb->estado = READY;

	pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready_prioritario);
	queue_push(array_queues[pcb->prioridad].queue_ready_prioritario, pcb);
	pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready_prioritario);

	log_cambio_estado(pcb, EXEC, READY); // ! NO HACE FALTA HACER EL SEM_POST PORQUE NO SE PUEDE ALOJAR NADA PORQUE SE DESCONECTA LA CPU
}

void ks_termina_de_desalojar_por_compactacion(void) {
	pthread_mutex_lock(&mutex_exec);

	for (int i = 0; i < list_size(procs_exec); i++) {
		t_pcb* pcb = list_get(procs_exec, i);

		pcb->estado = READY;

		pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_ready_prioritario);
		queue_push(array_queues[pcb->prioridad].queue_ready_prioritario, pcb);
		pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_ready_prioritario);

		log_cambio_estado(pcb, EXEC, READY); // ! NO HACE FALTA HACER EL SEM_POST PORQUE NO SE PUEDE ALOJAR NADA DURANTE LA COMPACTACIÓN
	}
	// list_clean(procs_exec);

	pthread_mutex_unlock(&mutex_exec);
}
