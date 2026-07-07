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
    char *puerto_kernel_memory;
    char *puerto_kernel_scheduler;
    t_log_level log_level;

} cpu_struct_config;

// Declaración Variables Globales:
extern t_log *cpu_logger;
extern cpu_struct_config cpu_config;
extern int conexion_km;
extern int conexion_ks;
extern int identificador_cpu;
extern int PID_a_ejecutar;
extern int PID_Interrupcion;
extern t_list *lista_memory_sticks;
extern int flag_syscall;
extern int flag_seg_fault; // se setea al detectar SEG_FAULT: corta el ciclo del proceso sin matar la CPU
extern int flag_pc_modificado; // 1 si la instrucción escribió PC explícitamente (JNZ/SET/MOV a PC): no aplicar PC++
extern int TAM_MAX_SEGMENTO;
extern t_contexto_ejecucion contexto_aux_global;
extern t_instruccion instruccion_aux_global;
//  Semaforos:
extern sem_t sem_proceso_nuevo;
extern sem_t sem_contexto_recibido;
extern sem_t sem_contexto_procesado;
extern sem_t sem_instruccion_recibida;
extern sem_t sem_instruccion_procesada;
extern sem_t hay_interrupcion;
extern sem_t semaphore;                    // Coordinacion //POR AHORA NO SE USA
extern pthread_mutex_t mutex_lista_sticks; // MUTEX
extern pthread_mutex_t conexiones_km;      // MUTEX
extern pthread_mutex_t conexiones_ks;      // MUTEX

#endif