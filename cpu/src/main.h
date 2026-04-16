#ifndef CPU_MAIN_H
#define CPU_MAIN_H
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
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
typedef struct
{
    char *IP_KERNEL_MEMORY;
    char *IP_KERNEL_SCHEDULER;
    char *IP_MEMORY_STICK;
    char *PUERTO_KERNEL_MEMORY;
    char *PUERTO_KERNEL_SCHEDULER;
    char *PUERTO_MEMORY_STICK;
    int SOCKET_KERNEL_MEMORY;
    int SOCKET_KERNEL_SCHEDULER;
    int SOCKET_MEMORY_STICK;
    char *LOG_LEVEL;
} cpu_struct_config;
extern cpu_struct_config cpu_config;
extern t_log *cpuLogger;

#endif