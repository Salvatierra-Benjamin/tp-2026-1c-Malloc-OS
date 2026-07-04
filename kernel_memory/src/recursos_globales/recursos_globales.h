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
    int prioridad;
} t_procesos_memoria;

typedef struct
{
    int cpu_id;
    int cpu_socket;
    pthread_mutex_t mutex_socket; // el socket lo escriben dos hilos: manejador_cpu (respuestas) y los hilos de stick (aviso_aumento_memoria)
} t_cpu;

typedef struct
{
    uint32_t base;
    uint32_t tamanio;
} t_hueco;

// Conexión de datos dedicada hacia un Memory Stick (local a KM: no tocar t_memory_stick,
// que también lo usa CPU). base/tamanio se copian del stick al conectar y son inmutables.
typedef struct
{
    uint32_t base;
    uint32_t tamanio;
    int socket_datos;            // nunca se cierra: el stick hace exit() si un cliente se desconecta
    pthread_mutex_t mutex_datos; // protege el par completo enviar+recibir sobre socket_datos
} t_datos_ms;

extern t_list *lista_huecos;
extern pthread_mutex_t mutex_lista_huecos;

extern t_list *lista_cpu;
extern pthread_mutex_t mutex_lista_cpu;

extern t_dictionary *procesos_memoria;
extern pthread_mutex_t mutex_procesos_memoria;

extern t_list *lista_memory_sticks;
extern pthread_mutex_t mutex_lista_memory_sticks;

extern t_list *lista_datos_ms; // t_datos_ms [] — conexiones de datos, paralela a lista_memory_sticks
extern pthread_mutex_t mutex_lista_datos_ms;

extern t_list *lista_procesos_suspendidos;
extern pthread_mutex_t mutex_lista_procesos_suspendidos;

extern int socket_kernel_scheduler;
extern pthread_mutex_t mutex_socket_kernel_scheduler;

extern int socket_swap;
extern pthread_mutex_t mutex_socket_swap;

extern t_log *logger;
extern t_config *config;
extern km_config *parametros;

void iniciar_recursos();
void liberar_recursos();

void destruir_instruccion(void *elem);
void destruir_contexto(t_contexto_ejecucion *ctx);
void destruir_proceso_memoria(void *elem);

#endif