#ifndef MANEJADOR_CPU_H
#define MANEJADOR_CPU_H

#include <recursos_globales/recursos_globales.h>

void *manejador_cpu(void *arg);
void conexion_cpu(int cpu_id);
void solicitar_contexto(int pid);

#endif