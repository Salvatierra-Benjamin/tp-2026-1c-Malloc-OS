#ifndef MANEJADOR_CPU_H
#define MANEJADOR_CPU_H

#include <recursos_globales/recursos_globales.h>

void *manejador_cpu(void *arg);
void conexion_cpu(int cpu_id);
t_contexto_ejecucion *solicitar_contexto(int pid);
t_instruccion *solicitar_instruccion(int pid, uint32_t pc);

#endif