#include "conexiones_swap.h"
void creacion_conexiones_swap()
{
    conexion_km = crear_conexion(swap_config.ip_km, swap_config.puerto_km);
    if (conexion_km == -1)
    {
        log_info(swap_logger, "ERROR AL CREAR CONEXION SWAP->KM.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_km, HANDSHAKE_SWAP);
    receptor_km();
}
