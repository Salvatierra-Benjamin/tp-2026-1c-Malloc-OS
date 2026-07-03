#include "escritura.h"

void procesar_peticion_escritura(int socket, uint32_t direccion_fisica, void *datos, int tamanio_a_escribir)
{

    usleep(ms_config.memory_delay * 1000);

    memcpy((uint8_t *)puntero_memoria_ms + direccion_fisica, datos, tamanio_a_escribir);
}