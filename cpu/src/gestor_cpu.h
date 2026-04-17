#ifndef GESTORS_CPU_H
#define GESTORS_CPU_H
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

typedef struct
{
    char *ip_kernel_memory;
    char *ip_kernel_scheduler;
    char *ip_memory_stick;
    char *puerto_kernel_memory;
    char *puerto_kernel_scheduler;
    char *puerto_memory_stick;
    t_log_level log_level;

} cpu_struct_config;

extern t_log *cpu_logger;
extern cpu_struct_config cpu_config;

#endif