#ifndef KS_AUXILIARES_H
#define KS_AUXILIARES_H

#include "../estructuras/ks_estructuras.h"
#include "../logs/ks_logs.h"

#include <stdlib.h>
#include <string.h>

#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// LIBERACION
void liberar_proceso(void* proc);
void liberar_proceso_bsod(void* proc);

void matar_procesos_ready(void);

void liberar_listas(void);
void liberar_mutex(void);

void ks_liberar_array_queues(void);

// PCB
bool finalizar_proceso(int pid);

t_pcb* crear_proceso(int pid, char* path, int prioridad);

t_pcb* buscar_pcb_por_pid(int pid);

#endif