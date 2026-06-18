#include "escritura.h"

void procesar_peticion_escritura(int socket, uint32_t direccion_fisica, void *datos, int tamanio_a_escribir)
{

    // TODO Validar en cpu/KM que no salga del limite (aka seg fault)
    // if (direccion_fisica + tamanio_a_escribir > (uint32_t)tamanio_memoria_ms_ingresada)
    // {
    //     log_error(ms_logger, "Intento de escritura fuera de límites: dir=%u tam=%d tamaño_memoria=%d",
    //               direccion_fisica, tamanio_a_escribir, tamanio_memoria_ms_ingresada);

    //     int estado_error = 0;
    //     t_paquete *paquete_error = crear_paquete(RESPUESTA_ESCRITURA_MS);
    //     agregar_a_paquete(paquete_error, &estado_error, sizeof(int));
    //     enviar_paquete(paquete_error, socket);
    //     eliminar_paquete(paquete_error);
    //     exit(EXIT_FAILURE);
    // }
    //! DELAY FUTURO
    memcpy((uint8_t *)puntero_memoria_ms + direccion_fisica, datos, tamanio_a_escribir);
}