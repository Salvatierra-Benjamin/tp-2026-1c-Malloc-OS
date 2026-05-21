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

// SUSPENSIONES
void ks_suspendar_proceso(t_pcb* pcb);

void ks_reanudar_proceso(t_pcb* pcb);

// AUXILIARES
bool ks_supera_tiempo_suspension(t_pcb* pcb);

#endif