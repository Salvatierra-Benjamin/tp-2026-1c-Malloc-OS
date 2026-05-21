#ifndef KS_PLANIFICADOR_LARGO_H
#define KS_PLANIFICADOR_LARGO_H

#include "../estructuras/ks_estructuras.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// PLANIFICADOR
void* ks_planificador_largo_plazo(void* arg);

// TRANSICIONES
void ks_admitir_proceso(t_pcb* pcb);

void ks_finalizar_proceso_planificador(t_pcb* pcb);

// AUXILIARES
bool ks_hay_espacio_memoria(void);

#endif