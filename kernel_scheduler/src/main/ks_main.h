#ifndef KS_MAIN_H
#define KS_MAIN_H

#include "../estructuras/ks_estructuras.h"

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>

#include <../../utils/src/utils/conexiones.h>
#include <../../utils/src/utils/protocolos.h>

// CONFIG
bool ks_cargar_config(char* path_config, char* path_proceso_inicial);
void ks_liberar_config(void);

// ALGORITMOS
void ks_parsear_algoritmos(char* raw);

// CONEXIONES
void* ks_conexion_loop(void* arg);
void* ks_conexion_km(void* arg);

// LISTAS
void crear_listas(void);

#endif