#ifndef RECURSOS_GLOBALES
#define RECURSOS_GLOBALES

#include <utils/conexiones.h>
#include <utils/protocolos.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/string.h>

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

typedef struct
{
    t_list *instrucciones; // t_instruccion []
    t_contexto_ejecucion *contexto;
} t_procesos_memoria;

typedef struct
{
    int base;
    int tamanio;
    int puerto;
    int socket;
} t_memory_stick;

extern t_dictionary *procesos_memoria;
extern pthread_mutex_t mutex_procesos_memoria;

extern t_list *lista_memory_sticks;
extern pthread_mutex_t mutex_lista_memory_sticks;

extern t_log *logger;
extern t_config *config;
extern km_config *parametros;

void iniciar_recursos();
void liberar_recursos();

void destruir_instruccion(void *elem);
void destruir_contexto(t_contexto_ejecucion *ctx);
void destruir_proceso_memoria(void *elem);

#endif