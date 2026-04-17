#ifndef RECEPTOR_CPU_H
#define RECEPTOR_CPU_H

#include "gestor_ms.h"
void esperar_cpus(int socket_servidor);
void *atencion_cpus(void *socket_cliente_puntero);
#endif