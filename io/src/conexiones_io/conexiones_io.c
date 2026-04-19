#include "conexiones_io.h"
void creacion_conexiones_io()
{
    conexion_ks = crear_conexion(io_config.ip_ks, io_config.puerto_ks);
    if (conexion_ks == -1)
    {
        log_info(io_logger, "ERROR AL CREAR CONEXION IO->KS.\n");
        io_liberar_config();
        io_liberar_log();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_ks, HANDSHAKE_IO);
    receptor_ks();
}
