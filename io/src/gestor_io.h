#ifndef GESTORS_IO_H
#define GESTORS_IO_H
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

// Variables globales
typedef struct
{
    t_log_level log_level;
    char *ip_ks;
    char *puerto_ks;
} io_config_struct;

// Variables globales
extern t_log *io_logger;
extern io_config_struct io_config;
#endif
