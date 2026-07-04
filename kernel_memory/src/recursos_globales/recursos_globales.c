#include "recursos_globales.h"

t_dictionary *procesos_memoria;

pthread_mutex_t mutex_procesos_memoria;

t_list *lista_memory_sticks;
pthread_mutex_t mutex_lista_memory_sticks;

t_list *lista_datos_ms;
pthread_mutex_t mutex_lista_datos_ms;

int socket_kernel_scheduler;
pthread_mutex_t mutex_socket_kernel_scheduler;

int socket_swap;
pthread_mutex_t mutex_socket_swap;

int tam_bloque_swap;
int cantidad_bloques_swap;
bool *bloques_swap_ocupados;
pthread_mutex_t mutex_bloques_swap;

t_list *lista_procesos_suspendidos;
pthread_mutex_t mutex_lista_procesos_suspendidos;

t_list *lista_cpu;
pthread_mutex_t mutex_lista_cpu;

t_list *lista_huecos;
pthread_mutex_t mutex_lista_huecos;

void iniciar_recursos()
{
    pthread_mutex_init(&mutex_procesos_memoria, NULL);
    procesos_memoria = dictionary_create();

    pthread_mutex_init(&mutex_lista_memory_sticks, NULL);
    lista_memory_sticks = list_create();

    pthread_mutex_init(&mutex_lista_datos_ms, NULL);
    lista_datos_ms = list_create();

    pthread_mutex_init(&mutex_socket_kernel_scheduler, NULL);
    socket_kernel_scheduler = -1;

    pthread_mutex_init(&mutex_socket_swap, NULL);
    socket_swap = -1;

    pthread_mutex_init(&mutex_bloques_swap, NULL);
    tam_bloque_swap = 0;
    cantidad_bloques_swap = 0;
    bloques_swap_ocupados = NULL;

    pthread_mutex_init(&mutex_lista_cpu, NULL);
    lista_cpu = list_create();

    pthread_mutex_init(&mutex_lista_huecos, NULL);
    lista_huecos = list_create();

    pthread_mutex_init(&mutex_lista_procesos_suspendidos, NULL);
    lista_procesos_suspendidos = list_create();
}

static void destruir_memory_stick(void *elem)
{
    t_memory_stick *ms = (t_memory_stick *)elem;
    free(ms->ip);
    free(ms->puerto);
    free(ms);
}

static void destruir_datos_ms(void *elem)
{
    t_datos_ms *datos = (t_datos_ms *)elem;
    pthread_mutex_destroy(&datos->mutex_datos);
    free(datos);
}

static void destruir_proceso_suspendido(void *elem)
{
    t_proceso_suspendido *ps = (t_proceso_suspendido *)elem;
    destruir_proceso_memoria(ps->proceso);
    list_destroy_and_destroy_elements(ps->bloques, free);
    free(ps);
}

static void destruir_cpu(void *elem)
{
    t_cpu *cpu = (t_cpu *)elem;
    pthread_mutex_destroy(&cpu->mutex_socket);
    free(cpu);
}

void liberar_recursos()
{
    pthread_mutex_destroy(&mutex_procesos_memoria);
    pthread_mutex_destroy(&mutex_lista_memory_sticks);
    pthread_mutex_destroy(&mutex_lista_datos_ms);
    pthread_mutex_destroy(&mutex_socket_kernel_scheduler);
    pthread_mutex_destroy(&mutex_socket_swap);
    pthread_mutex_destroy(&mutex_bloques_swap);
    pthread_mutex_destroy(&mutex_lista_cpu);
    pthread_mutex_destroy(&mutex_lista_huecos);
    pthread_mutex_destroy(&mutex_lista_procesos_suspendidos);

    free(bloques_swap_ocupados);

    free(parametros->allocation_strategy);
    free(parametros->puerto_escucha);
    free(parametros->scripts_basepath);
    free(parametros);
    dictionary_destroy_and_destroy_elements(procesos_memoria, destruir_proceso_memoria);

    list_destroy_and_destroy_elements(lista_memory_sticks, destruir_memory_stick);
    list_destroy_and_destroy_elements(lista_datos_ms, destruir_datos_ms);
    list_destroy_and_destroy_elements(lista_cpu, destruir_cpu);
    list_destroy_and_destroy_elements(lista_huecos, free);
    list_destroy_and_destroy_elements(lista_procesos_suspendidos, destruir_proceso_suspendido);

    log_destroy(logger);
    config_destroy(config);
}

void destruir_instruccion(void *elem)
{
    t_instruccion *i = (t_instruccion*)elem;
    free(i->parametro_1);
    free(i->parametro_2);
    free(i);
}

void destruir_contexto(t_contexto_ejecucion *ctx)
{
    free(ctx->tabla_segmentos.segmento);
    free(ctx);
}

void destruir_proceso_memoria(void *elem)
{
    t_procesos_memoria *p = (t_procesos_memoria*)elem;
    list_destroy_and_destroy_elements(p->instrucciones, destruir_instruccion);
    destruir_contexto(p->contexto);
    free(p);
}
