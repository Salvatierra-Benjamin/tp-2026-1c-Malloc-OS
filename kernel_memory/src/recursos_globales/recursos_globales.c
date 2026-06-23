#include "recursos_globales.h"

t_dictionary *procesos_memoria;

pthread_mutex_t mutex_procesos_memoria;

t_list *lista_memory_sticks;
pthread_mutex_t mutex_lista_memory_sticks;

int socket_kernel_scheduler;
pthread_mutex_t mutex_socket_kernel_scheduler;

t_list *lista_cpu;
pthread_mutex_t mutex_lista_cpu;

void iniciar_recursos()
{
    pthread_mutex_init(&mutex_procesos_memoria, NULL);
    procesos_memoria = dictionary_create();

    pthread_mutex_init(&mutex_lista_memory_sticks, NULL);
    lista_memory_sticks = list_create();

    pthread_mutex_init(&mutex_socket_kernel_scheduler, NULL);
    socket_kernel_scheduler = -1;

    pthread_mutex_init(&mutex_lista_cpu, NULL);
    lista_cpu = list_create();
}

void liberar_recursos()
{
    pthread_mutex_destroy(&mutex_procesos_memoria);
    pthread_mutex_destroy(&mutex_lista_memory_sticks);
    pthread_mutex_destroy(&mutex_socket_kernel_scheduler);
    pthread_mutex_destroy(&mutex_lista_cpu);

    free(parametros->allocation_strategy);
    free(parametros->puerto_escucha);
    free(parametros->scripts_basepath);
    free(parametros);
    dictionary_destroy_and_destroy_elements(procesos_memoria, destruir_proceso_memoria);

    list_destroy_and_destroy_elements(lista_memory_sticks, free);
    list_destroy_and_destroy_elements(lista_cpu, free);

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
