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

// Definición de Tipos:
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

// Declaración Variables Globales:
extern t_log *cpu_logger;
extern cpu_struct_config cpu_config;
extern int conexion_km;
extern int conexion_ks;
extern int conexion_ms;
extern int identificador_cpu;
extern sem_t sem_proceso_nuevo;
extern sem_t hay_interrupcion;
extern sem_t notificar_ks;
extern int PID_a_ejecutar;

//  Semaforos:
extern pthread_mutex_t conexiones_km; // MUTEX
extern pthread_mutex_t conexiones_ks; // MUTEX
extern pthread_mutex_t conexiones_ms; // MUTEX
extern sem_t semaphore;               // Coordinacion //POR AHORA NO SE USA

#endif