#include "ks_planificador_largo.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"
#include "../auxiliares/ks_auxiliares.h"

// void* ks_planificador_largo_plazo(void* arg) {
//     (void)arg;

//     while (1) {
//         sem_wait(&sem_memoria_disponible);

//         for (int i = 0; i < cantidad_algoritmos; i++) {

//             pthread_mutex_lock(&array_queues[i].mutex_susp_ready);

//             while (!queue_is_empty(array_queues[i].queue_susp_ready)) {
//                 t_pcb* pcb = queue_pop(array_queues[i].queue_susp_ready);

//                 pcb->estado = READY;

//                 pthread_mutex_lock(&array_queues[i].mutex_ready);

//                 queue_push(array_queues[i].queue_ready,pcb);

//                 pthread_mutex_unlock(&array_queues[i].mutex_ready);

//                 sem_post(&sem_ready);
//             }

//             pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
//         }
//     }

//     return NULL;
// }