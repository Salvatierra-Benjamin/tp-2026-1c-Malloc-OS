#ifndef KS_ESTRUCTURAS_H
#define KS_ESTRUCTURAS_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>

// Config

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

// STRUCT QUE VA EN EL ARRAY DE ALGORITMOS
typedef struct {
    algoritmos alg;
    t_queue* queue_ready;
} t_elemento_array_algoritmos;

// CONFIG
typedef struct {
    char *ip_km;
    char *puerto_km;
    char *puerto_escucha;
    algoritmos algoritmo;     // FIFO, RR, CMN
    // t_list *algoritmo_queues; // lista entre FIFO, RR // LO COMENTAMOS PARA VER COMO ANDABA EQUIPO CPU
    bool queue_preemption;
    // char* queue_preemption; // convertir.a bool
    int rr_quantum;
    int suspension_timeout;
    char* path_proceso_inicial;
    t_log_level log_level;
} t_ks_config;

typedef enum {
    NEW = 0,
    READY = 1,
    EXEC = 2,
    BLOCK = 3,
    SUSP_BLOCK = 4,
    SUSP_READY = 5,
    EXIT = 6
} estados;

/* //! Pongo el enum para despues hacer:
algoritmos alg = (algoritmos)(intptr_t)list_get(ks_config.algoritmo_queues, i);

switch (alg) {
    case FIFO: ...
    case RR: ...
}
*/

// PROCESO
typedef struct {
    int pid;
    char* path;
    int prioridad;
    int cpu_fd; // socket del CPU al que se le asignó el proceso (EXEC), o -1 si no está asignado
    int io_fd; // socket del IO, o -1 si no está asignado
    estados estado;
} t_pcb;


// Variables globales
extern t_log *ks_logger;
extern t_ks_config ks_config;
extern t_elemento_array_algoritmos* array_queues;
extern int cantidad_algoritmos;

// LISTAS DE CONEXIONES
extern t_list* cpus = NULL;
extern t_list* ios = NULL;

// LISTAS DE ESTADOS
extern t_list* procs_exec = NULL;
extern t_list* procs_block = NULL;
extern t_list* procs_susp_block = NULL;
extern t_list* procs_susp_ready = NULL;
extern t_list* procs_exit = NULL;

#endif