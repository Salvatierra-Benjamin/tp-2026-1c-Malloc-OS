#ifndef RECURSOS_GLOBALES
#define RECURSOS_GLOBALES

#include <utils/conexiones.h>
#include <utils/protocolos.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>

typedef struct
{
    char *puerto_escucha;
    t_log_level log_level;
    int segment_max_size;
    char *allocation_strategy;
    int instruction_delay;
    int compaction_delay;
    char *scripts_basepath;
} km_config;

// propuesta estructuras para contexto de ejecucion
/*
typedef struct
{
    t_funcion *func; // enum con instrucciones
    char *param1;
    char *param2;
} t_instruccion;

typedef struct
{
    int pc, ax, bx, cx, dx, eax, ebx, ecx, edx, si, di;
} t_registros_cpu;

typedef struct
{
    t_registros_cpu registros;
    t_list *tabla_segmentos;
} t_contexto_ejecucion;

typedef struct
{
    t_list *instrucciones;
    t_contexto_ejecucion *contexto;
} t_procesos_memoria;

extern t_dictionary *procesos_memoria;
pthread_mutex_t mutex_procesos;
*/
// fin propuesta

extern t_log *logger;
extern t_config *config;
extern km_config *parametros;

void liberar_recursos();

#endif