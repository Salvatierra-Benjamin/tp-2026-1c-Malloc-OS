#ifndef KS_MAIN_H
#define KS_MAIN_H

#include "ks_estructuras.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#include <../../utils/src/utils/conexiones.h>
#include <../../utils/src/utils/protocolos.h>

// VARIABLES GLOBALES
t_log* ks_logger = NULL;
t_ks_config ks_config;
int ks_server_fd = -1;

// FUNCIONES
// auxiliares
t_list* ks_parsear_algoritmos(char* raw);

// config
bool ks_cargar_config(char* path);
void ks_liberar_config(void);

// conexiones
void* ks_conexion_loop(void* arg);
void* ks_conexion_km(void* arg);

#endif