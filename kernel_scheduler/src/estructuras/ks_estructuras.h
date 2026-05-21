#ifndef KS_ESTRUCTURAS_H
#define KS_ESTRUCTURAS_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// CPU
typedef struct {
    uint32_t id;
    uint32_t running_qid;
    int fd;
} t_cpu;

// IO
typedef struct {
    char* path;
    uint32_t prio;
    int fd;
} t_io;

// ENUM ALGORITMOS
typedef enum {
    FIFO = 1,
    RR = 2,
    CMN = 3,
    INVALIDO = 0
} algoritmos;

// ARRAY DE ALGORITMOS
typedef struct {

    algoritmos alg;

    t_queue* queue_ready;
    pthread_mutex_t mutex_ready;

    t_queue* queue_block;
    pthread_mutex_t mutex_block;

    t_queue* queue_susp_block;
    pthread_mutex_t mutex_susp_block;

    t_queue* queue_susp_ready;
    pthread_mutex_t mutex_susp_ready;

} t_elemento_array_algoritmos;

// CONFIG
typedef struct {

    char *ip_km;
    char *puerto_km;
    char *puerto_escucha;

    algoritmos algoritmo;

    bool queue_preemption;

    int rr_quantum;
    int suspension_timeout;

    char* path_proceso_inicial;

    t_log_level log_level;

} t_ks_config;

// ESTADOS
typedef enum {
    NEW = 0,
    READY = 1,
    EXEC = 2,
    BLOCK = 3,
    SUSP_BLOCK = 4,
    SUSP_READY = 5,
    EXIT = 6
} estados;

// PCB
typedef struct {

    int pid;
    char* path;

    int prioridad;

    int cpu_fd;
    int io_fd;

    estados estado;

} t_pcb;

// VARIABLES GLOBALES
extern t_log *ks_logger;
extern t_ks_config ks_config;

extern int ks_server_fd;

extern t_elemento_array_algoritmos* array_queues;
extern int cantidad_algoritmos;

// LISTAS
extern t_list* cpus;
extern t_list* ios;

extern t_list* procs_exec;
extern t_list* procs_block;
extern t_list* procs_susp_block;
extern t_list* procs_susp_ready;
extern t_list* procs_exit;

// MUTEX
extern pthread_mutex_t mutex_cpus;
extern pthread_mutex_t mutex_ios;

extern pthread_mutex_t mutex_exec;
extern pthread_mutex_t mutex_exit;

extern pthread_mutex_t mutex_array_queues;

#endif