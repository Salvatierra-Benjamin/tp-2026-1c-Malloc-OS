#ifndef KS_PLANIFICADOR_CORTO_H
#define KS_PLANIFICADOR_CORTO_H

#include "../estructuras/ks_estructuras.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// PLANIFICADOR
void* ks_planificador_corto_plazo(void* arg);

// ALGORITMOS
void ks_planificar_fifo(void);
void ks_planificar_rr(void);
void ks_planificar_cmn(void);

// AUXILIARES
t_pcb* ks_obtener_siguiente_ready(void);

bool ks_hay_procesos_ready(void);

void ks_enviar_a_exec(t_pcb* pcb, int cpu_fd);

#endif