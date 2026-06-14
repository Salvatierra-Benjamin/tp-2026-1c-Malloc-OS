#ifndef OPERACIONES_H
#define OPERACIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "../iniciador_swap.h"

void escribir_bloque(int numero_bloque, void *datos);
int leer_bloque(int numero_bloque, void *buffer);

#endif // OPERACIONES_H