#ifndef OPERACIONES_H
#define OPERACIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "../iniciador_swap.h"

bool bloque_valido(int numero_bloque);
int escribir_bloque(int numero_bloque, void *datos);
int leer_bloque(int numero_bloque, void *buffer);

#endif // OPERACIONES_H