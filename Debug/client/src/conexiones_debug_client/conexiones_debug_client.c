#include "conexiones_debug_client.h"

// Retorna true si el servidor aceptó la conexión, false si hubo un error
bool realizar_handshake(int socket_servidor, int id_debug_client, char *nombre_servidor)
{
    // 1. Enviamos nuestro ID de debug_client
    t_paquete *paquete_handshake = crear_paquete(HANDSHAKE_DEBUG_CLIENT);
    agregar_a_paquete(paquete_handshake, &id_debug_client, sizeof(int));
    enviar_paquete(paquete_handshake, socket_servidor);
    eliminar_paquete(paquete_handshake);

    log_info(debug_client_logger, "Handshake enviado a %s. Esperando respuesta...", nombre_servidor);

    // 2. Esperamos la confirmación sincrónica
    int respuesta = recibir_operacion(socket_servidor);

    if (respuesta == HANDSHAKE_OK)
    {
        log_info(debug_client_logger, "Handshake con %s exitoso.", nombre_servidor);
        return true;
    }
    else
    {
        log_error(debug_client_logger, "Falló el handshake con %s. Código recibido: %d", nombre_servidor, respuesta);
        return false;
    }
}

void creacion_conexiones_debug_client()
{
    conexion_debug_server = crear_conexion(debug_client_config.ip_debug_server, debug_client_config.puerto_debug_server);
    if (conexion_debug_server == -1)
    {
        log_error(debug_client_logger, "ERROR AL CREAR CONEXION debug_client->debug_serconexion_debug_server.");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    if (!realizar_handshake(conexion_debug_server, identificador_debug_client, "HANDSHAKE DEBUG SERVER"))
    {
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    receptor_debug_server();
}