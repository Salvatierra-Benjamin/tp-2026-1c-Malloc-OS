#ifndef GESTORS_DEBUG_SERVER_H
#define GESTORS_DEBUG_SERVER_H
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include "../../../utils/src/utils/conexiones.h"
#include "../../../utils/src/utils/protocolos.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

// Definición de Tipos:
typedef struct
{
    char *puerto_escucha_debug_client;
    t_log_level log_level;
} debug_server_config_struct;

// Declaración Variables Globales:
extern t_log *debug_server_logger;
extern debug_server_config_struct debug_server_config;
extern int conexion_debug_server;
extern int socket_servidor_con_debug_client;
extern int tamanio_memoria;
extern void *memoria_debug_server;
extern sem_t semaphore; // Coordinacion //POR AHORA NO SE USA
extern int PID_a_ejecutar;

#endif