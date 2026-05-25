#include "conexiones_debug_server.h"
void creacion_conexiones_debug_server()
{
    socket_servidor_con_debug_client = iniciar_servidor(debug_server_config.puerto_escucha_debug_client);
    if (socket_servidor_con_debug_client == -1)
    {
        log_info(debug_server_logger, "ERROR AL CREAR CONEXION debug_client->debug_server.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    esperar_debug_client();
}
