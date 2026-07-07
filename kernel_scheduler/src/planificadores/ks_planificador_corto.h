#ifndef KS_PLANIFICADOR_CORTO_H
#define KS_PLANIFICADOR_CORTO_H

#include "../estructuras/ks_estructuras.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>

// PLANIFICADOR
void* ks_planificador_corto_plazo(void* arg);

// READY
t_pcb* ks_obtener_siguiente_ready(void);
t_pcb* ks_obtener_siguiente_ready_prioritario(void);

// ALGORITMOS
void ks_planificar_fifo(t_pcb* pcb);
void ks_planificar_rr(t_pcb* pcb);
void ks_planificar_cmn(t_pcb* pcb);

// EXEC
t_cpu* ks_enviar_a_exec(t_pcb* pcb); // devuelve la CPU asignada, o NULL si no se pudo despachar
void ks_verificar_desalojo_prioridad(t_pcb* nuevo);

// DESALOJOS Y COMPACTACION
void ks_empieza_a_desalojar_por_compactacion(void);
void ks_termina_de_desalojar_un_pcb_general(t_pcb* pcb, bool a_ready_prioritario);
void ks_reubicar_ready_por_cambio_prioridad(t_pcb* pcb, int prioridad_vieja, int prioridad_nueva);
void ks_termina_de_desalojar_por_compactacion(void);
void ks_confirmar_desalojo_compactacion_de_una_cpu(void);

#endif