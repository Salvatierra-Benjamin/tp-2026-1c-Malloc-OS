#ifndef MANEJADOR_CPU_H
#define MANEJADOR_CPU_H

#include <recursos_globales/recursos_globales.h>

void *manejador_cpu(void *arg);
t_cpu *conexion_cpu(int cpu_id, int cpu_socket);

// Devuelven una COPIA hecha bajo mutex_procesos_memoria (el original puede ser
// realloc-eado por crear_segmento o liberado por finalizar_proceso en paralelo).
// Liberar con destruir_contexto / destruir_instruccion.
t_contexto_ejecucion *solicitar_contexto(int pid);
t_instruccion *solicitar_instruccion(int pid, uint32_t pc);

#endif