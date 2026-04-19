#ifndef KS_ESTRUCTURAS_H
#define KS_ESTRUCTURAS_H

#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>

// Config

// CPU
typedef struct
{
    uint32_t id;
    uint32_t running_qid;
    int fd;
} t_cpu;

// IO
typedef struct
{
    char *path;
    uint32_t prio;
    int fd;
} t_io;

typedef enum
{
    FIFO = 1,
    RR = 2,
    CMN = 3,
    INVALIDO = 0
} algoritmos;

typedef struct
{
    char *ip_km;
    char *puerto_km;
    char *puerto_escucha;
    algoritmos *algoritmo; // FIFO, RR, CMN
    // t_list *algoritmo_queues; // lista entre FIFO, RR // LO COMENTAMOS PARA VER COMO ANDABA EQUIPO CPU
    bool queue_preemption;
    int suspension_timeout;
    t_log_level log_level;
} t_ks_config;

/* //! Pongo el enum para despues hacer:
algoritmos alg = (algoritmos)(intptr_t)list_get(ks_config.algoritmo_queues, i);

switch (alg) {
    case FIFO: ...
    case RR: ...
}
*/

// Variables globales
extern t_log *ks_logger;
extern t_ks_config ks_config;

#endif