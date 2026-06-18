#ifndef KS_LOGS_H
#define KS_LOGS_H

#include "../estructuras/ks_estructuras.h"

#include <commons/log.h>

void log_conexion_km(void);

void log_conexion_cpu(int id_cpu);

void log_creacion_proceso(t_pcb* pcb);

void log_syscall_recibida(t_pcb* pcb, char* nombre_syscall);

void log_cambio_estado(
    t_pcb* pcb,
    char* estado_anterior,
    char* estado_actual
);

void log_fin_io(t_pcb* pcb);

void log_mutex_tomado(t_pcb* pcb, char* nombre_mutex);

void log_mutex_liberado(t_pcb* pcb, char* nombre_mutex);

void log_cambio_prioridad(
    t_pcb* pcb,
    int prioridad_anterior,
    int prioridad_nueva
);

void log_desalojo_quantum(t_pcb* pcb);

void log_desalojo_prioridad(
    t_pcb* pcb_desalojado,
    t_pcb* pcb_nuevo
);

void log_inicio_compactacion(void);

void log_fin_compactacion(void);

void log_fin_proceso(t_pcb* pcb, char* motivo);

#endif