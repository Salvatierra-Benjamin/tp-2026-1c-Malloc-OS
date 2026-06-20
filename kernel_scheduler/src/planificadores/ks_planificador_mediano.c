#include "ks_planificador_mediano.h"

#include "../main/ks_main.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"

// BLOCK -> SUSP_BLOCK
// READY -> SUSP_READY

// TRANSICIONES
bool pasar_a_block(t_pcb *pcb) {
	pthread_mutex_lock(&mutex_exec);
	t_pcb *pcb_removido = list_remove_element(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	if ((pcb == NULL) || (pcb_removido == NULL)) {
		liberar_proceso(pcb);
		return false;
	}

	log_cambio_estado(pcb, (estados EXEC), (estados BLOCK));

	pcb->estado = BLOCK;
	pcb->tiempo_inicio_block = time(NULL);

	pthread_mutex_lock(&array_queues[pcb->prioridad].mutex_block);
	list_add(array_queues[pcb->prioridad].list_block, pcb);
	pthread_mutex_unlock(&array_queues[pcb->prioridad].mutex_block);

	pthread_t hilo_suspend;
	t_pcb *pcb_copia = pcb;
	pthread_create(&hilo_suspend, NULL, ks_timer_suspension, pcb_copia);
	pthread_detach(hilo_suspend);

	return true;
}

void ks_pasar_a_susp_block(t_pcb *pcb) {
	int prio = pcb->prioridad;

	// sacar de BLOCK
	pthread_mutex_lock(&array_queues[prio].mutex_block);
	list_remove_element(array_queues[prio].list_block, pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_block);

	// cambiar estado
	pcb->estado = SUSP_BLOCK;
	pcb->tiempo_inicio_suspendido = time(NULL);
	
    // meter en SUSP_BLOCK
	pthread_mutex_lock(&array_queues[prio].mutex_susp_block);
	list_add(array_queues[prio].list_susp_block,pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_susp_block);

	log_cambio_estado(pcb, (estados BLOCK), (estados SUSP_BLOCK));

	enviar_aviso_suspension_km(pcb->pid);

	// ? TODO: liberar estructuras --- Hay que hacerlo?
}
void ks_pasar_a_susp_ready(t_pcb *pcb) {
	int prio = pcb->prioridad;

	pthread_mutex_lock(&array_queues[prio].mutex_susp_block);
    list_remove_element(array_queues[prio].list_susp_block, pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_susp_block);

    pcb->estado = SUSP_READY;
    
	pthread_mutex_lock(&array_queues[prio].mutex_susp_ready);
	list_add(array_queues[prio].list_susp_ready, pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_susp_ready);

	log_cambio_estado(pcb, (estados SUSP_BLOCK), (estados SUSP_READY));
}
void ks_pasar_a_ready_desde_susp_ready(t_pcb *pcb) {
	int prio = pcb->prioridad;

	pthread_mutex_lock(&array_queues[prio].mutex_susp_ready);
	list_remove_element(array_queues[prio].list_susp_ready, pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_susp_ready);

    pcb->estado = READY;

	pthread_mutex_lock(&array_queues[prio].mutex_ready);
    queue_push(array_queues[prio].queue_ready, pcb);
	pthread_mutex_unlock(&array_queues[prio].mutex_ready);

	log_cambio_estado(pcb, (estados SUSP_READY), (estados READY));
}

void ks_pasar_a_ready_desde_susp_ready_pid(int pid, int prioridad) {
	bool _pcb_tiene_pid(void* ptr) {
		t_pcb* pcb = (t_pcb*) ptr;
		return pcb->id == pid;
	}

	t_pcb* pcb = NULL;
	pthread_mutex_lock(&array_queues[prioridad].mutex_susp_ready);
	pcb = list_remove_by_condition(array_queues[prioridad].list_susp_ready, _pcb_tiene_pid);
	pthread_mutex_unlock(&array_queues[prioridad].mutex_susp_ready);

    pcb->estado = READY;

	pthread_mutex_lock(&array_queues[prioridad].mutex_ready);
    queue_push(array_queues[prioridad].queue_ready, pcb);
	pthread_mutex_unlock(&array_queues[prioridad].mutex_ready);

	log_cambio_estado(pcb, (estados SUSP_READY), (estados READY));

	// ? iria acá un sem_post para avisar al planificador de que hay un proceso en READY nuevo??? Pero como el planificador de corto plazo está bloqueando esperando un sem_wait, no haría falta porque cuando el proceso de corto plazo haga el sem_wait va a desbloquearse igual aunque no haya un sem_post específico para esto
}

// TIMER DE SUSPENSION
void *ks_timer_suspension(void *arg) {
	t_pcb *pcb = (t_pcb *)arg;

	sleep(ks_config.suspension_timeout);

	// verificar si sigue bloqueado
	if (pcb->estado != BLOCK) {
		return NULL;
	}

	ks_pasar_a_susp_block(pcb);

	return NULL;
}

// DESUSPENSION
void ks_empezar_a_intentar_desuspender(void) {
	t_list* lista_a_desuspender = list_create();

    for (int i = 0; i < cantidad_algoritmos; i++) {

        while (1) {
			pthread_mutex_lock(&array_queues[i].mutex_susp_ready);

            if (list_is_empty(array_queues[i].list_susp_ready)) {
				pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
				log_debug(ks_logger, "No hay procesos en SUSP_READY para intentar desuspender en la prioridad %d", i);
                break;
            }

			t_pcb* pcb = list_get(array_queues[i].list_susp_ready, 0);

			list_add(lista_a_desuspender, pcb);

			pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
        }
    }

	// le envio la lista a KM para que intente desuspenderlos, y cuando me confirme cuales pudo desuspender, los paso a READY
	enviar_lista_desuspension_km(lista_a_desuspender);

	list_destroy(lista_a_desuspender);
}

// PASO A ESTAR EN KS_MANEJA_KM AL RECUBIR LA CONFIRMACIÓN
// void ks_terminar_de_intentar_desuspender(t_pcb* pcb) {
// 	t_pcb* pcb = queue_peek(array_queues[i].queue_susp_ready);

// 	bool hay_memoria = true; // TODO: consultar a KM si hay memoria para desuspender este proceso

// 	if (!hay_memoria) {
// 		pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
// 		break;
// 	}

// 	queue_pop(array_queues[i].queue_susp_ready);
// 	pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);

// 	ks_pasar_a_ready_desde_susp_ready(pcb);
// }

// PLANIFICADOR
void *ks_planificador_mediano_plazo(void *arg) {
	(void)arg;

	while (1) {
		sem_wait(&sem_memoria_disponible);

        ks_empezar_a_intentar_desuspender();
	}

	return NULL;
}
