#ifndef ESPERAR_CPU_H
#define ESPERAR_CPU_H

#include "gestor_ms.h"
#include "iniciador_ms.h"
#include "operaciones_ms/escritura.h"
#include "operaciones_ms/lectura.h"

void esperar_cpus();
void *atencion_cpus(void *socket_cliente_puntero);

#endif