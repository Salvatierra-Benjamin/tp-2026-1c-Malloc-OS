#include "lectura.h"


void procesar_peticion_lectura(int socket, uint32_t direccion_fisica, int tamanio_a_leer)
{
    
    void *datos_leidos = malloc(tamanio_a_leer);

   
    memcpy(datos_leidos, (uint8_t *)puntero_memoria_ms + direccion_fisica, tamanio_a_leer);


    t_paquete *paquete_respuesta = crear_paquete(RESPUESTA_LECTURA_MS); 

    
    agregar_a_paquete(paquete_respuesta, datos_leidos, tamanio_a_leer);

    usleep(ms_config.memory_delay * 1000);

   
    enviar_paquete(paquete_respuesta, socket);

    
    eliminar_paquete(paquete_respuesta);
    free(datos_leidos);
}
