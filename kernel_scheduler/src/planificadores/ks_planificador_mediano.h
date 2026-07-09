#ifndef KS_PLANIFICADOR_MEDIANO_H
#define KS_PLANIFICADOR_MEDIANO_H

#include "../estructuras/ks_estructuras.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

// PLANIFICADOR
void* ks_planificador_mediano_plazo(void* arg);

// TIMER DE SUSPENSION
void* ks_timer_suspension(void* arg);

// TRANSICIONES
bool pasar_a_block(t_pcb* pcb);
void ks_pasar_a_susp_block(t_pcb* pcb);
void ks_pasar_a_susp_ready(t_pcb* pcb);
void ks_pasar_a_ready_desde_susp_ready(t_pcb* pcb);
void ks_pasar_a_ready_desde_susp_ready_pid(int pid, int prioridad);
bool ks_completar_stdin_post_desusp(int pid);

// DESUSPENSION
void ks_empezar_a_intentar_desuspender(void);

#endif