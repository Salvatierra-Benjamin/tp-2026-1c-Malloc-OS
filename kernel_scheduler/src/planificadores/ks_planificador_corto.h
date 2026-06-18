#ifndef KS_PLANIFICADOR_CORTO_H
#define KS_PLANIFICADOR_CORTO_H

#include "../estructuras/ks_estructuras.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// PLANIFICADOR
void* ks_planificador_corto_plazo(void* arg);

// READY
t_pcb* ks_obtener_siguiente_ready(void);

// ALGORITMOS
void ks_planificar_fifo(t_pcb* pcb);
void ks_planificar_rr(t_pcb* pcb);
void ks_planificar_cmn(t_pcb* pcb);

// EXEC
void ks_enviar_a_exec(t_pcb* pcb);

// COMPACTACION
void ks_desalojar_por_compactacion(void);

#endif