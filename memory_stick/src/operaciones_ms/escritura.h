#ifndef ESCRITURA_MS_H
#define ESCRITURA_MS_H

#include "gestor_ms.h"
#include "iniciador_ms.h"
void procesar_peticion_escritura(int socket, uint32_t direccion_fisica, void *datos, int tamanio_a_escribir);

#endif