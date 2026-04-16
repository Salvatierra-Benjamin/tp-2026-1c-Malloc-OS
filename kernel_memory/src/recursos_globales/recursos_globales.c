#include "recursos_globales.h"

void liberar_recursos()
{
    log_destroy(logger);
    config_destroy(config);
    free(parametros->allocation_strategy);
    free(parametros->puerto_escucha);
    free(parametros->scripts_basepath);
    free(parametros);
}
