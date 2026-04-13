#ifndef RECURSOS_GLOBALES
#define RECURSOS_GLOBALES

#include <utils/conexiones.h>
#include <utils/protocolos.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>


typedef struct
{
    char* puerto_escucha;
    t_log_level log_level;
    int segment_max_size;
    char *allocation_strategy;
    int instruction_delay;
    int compaction_delay;
    char *scripts_basepath;
} km_config;

extern t_log *logger;
extern t_config *config;

#endif