#ifndef KS_PLANIFICADOR_MEDIANO_H
#define KS_PLANIFICADOR_MEDIANO_H

#include "../estructuras/ks_estructuras.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// PLANIFICADOR
void* ks_planificador_mediano_plazo(void* arg);

// TIMER DE SUSPENSION
void* ks_timer_suspension(void* arg);

// TRANSICIONES
void ks_pasar_a_susp_block(t_pcb* pcb);
void ks_pasar_a_susp_ready(t_pcb* pcb);
void ks_pasar_a_ready_desde_susp_ready(t_pcb* pcb);

// DESUSPENSION
void ks_intentar_desuspender(void);

#endif