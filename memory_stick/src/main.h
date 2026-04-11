#ifndef SWAP_H
#define SWAP_H
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



typedef struct {
    int memoria_delay; // FIFO, RR, CMN
    int ip_km; // lista entre FIFO, RR
    int puerto_km;
    t_log_level log_level;
} ms_config;

// MEMORY_DELAY=1500
// IP_KM = 1000
// PUERTO_KM = 1000
// LOG_LEVEL=INFO

    

#endif