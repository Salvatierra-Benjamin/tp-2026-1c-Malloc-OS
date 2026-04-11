#ifdef KERNEL_MEMORY_UTILS_H
#define KERNERL_MEMORY_UTILS_H
#include <utils/hello.h>
#include <utils/conexiones.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>

typedef struct
{
    char* puerto_escucha;
    char *log_level;
    int segment_max_size;
    char *allocation_strategy;
    int instruction_delay;
    int compaction_delay;
    char *scripts_basepath;
} km_config;

extern t_log *logger;

km_config *iniciar_config();
t_log *iniciar_logger(t_log_level log_level);

#endif