#include "recursos_globales.h"

void liberar_recursos()
{
    log_destroy(logger);
    config_destroy(config);
}
