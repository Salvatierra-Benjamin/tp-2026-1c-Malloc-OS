#ifndef KS_MAIN_H
#define KS_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
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


typedef struct {
    char* ip_kernel_memory;
    char* puerto_escucha;
    char* algoritmo; // FIFO, RR, CMN
    char* algoritmo_queues; // lista entre FIFO, RR
    int rr_quantum;
    bool  queue_preemption;
    int   suspension_timeout;
    t_log_level log_level;
} ks_config;

#endif