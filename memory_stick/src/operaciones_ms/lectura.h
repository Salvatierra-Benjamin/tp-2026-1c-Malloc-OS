#ifndef LECTURA_H
#define LECTURA_H

#include "gestor_ms.h"
#include "iniciador_ms.h"

void procesar_peticion_lectura(int socket, uint32_t direccion_fisica, int tamanio_a_leer);

#endif