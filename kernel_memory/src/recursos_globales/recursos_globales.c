#include "recursos_globales.h"

t_dictionary *procesos_memoria;

pthread_mutex_t mutex_procesos_memoria;

void iniciar_recursos()
{
    pthread_mutex_init(&mutex_procesos_memoria, NULL);
    procesos_memoria = dictionary_create();
}

void liberar_recursos()
{
    pthread_mutex_destroy(&mutex_procesos_memoria);

    free(parametros->allocation_strategy);
    free(parametros->puerto_escucha);
    free(parametros->scripts_basepath);
    free(parametros);
    dictionary_destroy_and_destroy_elements(procesos_memoria, destruir_proceso_memoria);

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
    list_destroy_and_destroy_elements(ctx->tabla_segmentos, free);
    free(ctx);
}

void destruir_proceso_memoria(void *elem)
{
    t_procesos_memoria *p = (t_procesos_memoria*)elem;
    list_destroy_and_destroy_elements(p->instrucciones, destruir_instruccion);
    destruir_contexto(p->contexto);
    free(p);
}
