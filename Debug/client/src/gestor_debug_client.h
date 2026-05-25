#ifndef GESTORS_DEBUG_CLIENT_H
#define GESTORS_DEBUG_CLIENT_H
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
    char *ip_debug_server;
    char *puerto_debug_server;
    t_log_level log_level;

} debug_client_struct_config;

// Declaración Variables Globales:
extern t_log *debug_client_logger;
extern debug_client_struct_config debug_client_config;
extern int conexion_debug_server;
extern int identificador_debug_client;
extern sem_t sem_proceso_nuevo;
extern int PID_a_ejecutar;

#endif