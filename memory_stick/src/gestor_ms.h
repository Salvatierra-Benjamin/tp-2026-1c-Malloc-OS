#ifndef GESTORS_SWAP_H
#define GESTORS_SWAP_H
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include <../../utils/src/utils/conexiones.h>
#include <../../utils/src/utils/protocolos.h>
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
    char *puerto_escucha_cpu;
    char *ip_km;     // FIFO, RR, CMN
    char *puerto_km; // lista entre FIFO, RR
    int memory_delay;
    t_log_level log_level;
} ms_config_struct;

// Declaración Variables Globales:
extern t_log *ms_logger;
extern ms_config_struct ms_config;

#endif