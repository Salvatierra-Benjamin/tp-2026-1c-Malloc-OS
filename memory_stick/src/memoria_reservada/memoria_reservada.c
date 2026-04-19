#include "memoria_reservada.h"

void memoria_reservada()
{
    memoria_ms = malloc(tamanio_memoria);
    if (memoria_ms == NULL)
    {
        log_debug(ms_logger, "Error al hacer malloc de la memoria principal.\n");
        exit(EXIT_FAILURE);
    }
    log_debug(ms_logger, "Memoria de %d bytes reservada con éxito.\n", tamanio_memoria);
}