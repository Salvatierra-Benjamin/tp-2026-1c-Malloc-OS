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

    log_destroy(logger);
    config_destroy(config);
    free(parametros->allocation_strategy);
    free(parametros->puerto_escucha);
    free(parametros->scripts_basepath);
    free(parametros);
    dictionary_destroy_and_destroy_elements(procesos_memoria, free);
}
