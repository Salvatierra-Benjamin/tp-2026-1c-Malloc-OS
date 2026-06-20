#include "lectura.h"
// Supongamos que esta es tu memoria real del Memory Stick (tu malloc gigante)
// que reservaste al iniciar el módulo.

void procesar_peticion_lectura(int socket, uint32_t direccion_fisica, int tamanio_a_leer)
{

    // if ((direccion_fisica + tamanio_a_leer) > tamanio_memoria_ms_ingresada)
    // {
    //     log_error(ms_logger, "Violación de segmento. Intento de lectura fuera de límites.");
    //      enviar_paqueteks()...
    //     exit(EXIT_FAILURE);
    // }

    // 1. Reservar un buffer temporal para guardar los datos que vamos a leer
    void *datos_leidos = malloc(tamanio_a_leer);

    // 2. LEER DE LA MEMORIA (La Magia)
    // Casteamos 'puntero_memoria_ms' a (uint8_t*) para que C sepa que nos queremos
    // mover byte por byte, y le sumamos la direccion_fisica como offset.
    memcpy(datos_leidos, (uint8_t *)puntero_memoria_ms + direccion_fisica, tamanio_a_leer);

    // 3. Armar el paquete de respuesta (usando tu lógica de protocolos)
    t_paquete *paquete_respuesta = crear_paquete(RESPUESTA_LECTURA_MS); // OP_Code inventado

    // 4. Agregar los datos leídos al paquete
    agregar_a_paquete(paquete_respuesta, datos_leidos, tamanio_a_leer);

    usleep(ms_config.memory_delay * 1000);

    // 5. Enviar de vuelta a la CPU que te lo pidió
    enviar_paquete(paquete_respuesta, socket);

    // 6. Limpiar la memoria dinámica para evitar leaks
    eliminar_paquete(paquete_respuesta);
    free(datos_leidos);
}