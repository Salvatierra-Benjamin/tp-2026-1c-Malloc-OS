#include "handshakes.h"
// Retorna true si el servidor aceptó la conexión, false si hubo un error
bool realizar_handshake(int socket_servidor, int id_cpu, char *nombre_servidor)
{
    // 1. Enviamos el HANDSHAKE de CPU a KM, KS y MS.
    enviar_operacion(socket_servidor, HANDSHAKE_CPU);

    log_info(cpu_logger, "Handshake enviado a %s. Esperando respuesta...", nombre_servidor);

    // 2. Esperamos la confirmación sincrónica
    int respuesta = recibir_operacion(socket_servidor);

    if (respuesta == HANDSHAKE_OK)
    {
        log_info(cpu_logger, "Handshake con %s exitoso.", nombre_servidor);
        // 3. Enviamos nuestro ID de CPU
        t_paquete *paquete_handshake = crear_paquete(ENVIO_CPU_ID);
        agregar_a_paquete(paquete_handshake, &id_cpu, sizeof(int));
        enviar_paquete(paquete_handshake, socket_servidor);
        eliminar_paquete(paquete_handshake);

        return true;
    }
    else
    {
        log_error(cpu_logger, "Falló el handshake con %s. Código recibido: %d", nombre_servidor, respuesta);
        return false;
    }
}

// Retorna true si el servidor aceptó la conexión, false si hubo un error
bool realizar_handshake_ks(int socket_servidor, int id_cpu, char *nombre_servidor)
{
    // 1. Enviamos el HANDSHAKE de CPU a KS
    t_paquete *paquete_handshake = crear_paquete(HANDSHAKE_CPU);
    agregar_a_paquete(paquete_handshake, &id_cpu, sizeof(int));
    enviar_paquete(paquete_handshake, socket_servidor);
    eliminar_paquete(paquete_handshake);

    log_info(cpu_logger, "Handshake enviado a %s. Esperando respuesta...", nombre_servidor);

    // 2. Esperamos la confirmación sincrónica
    int respuesta = recibir_operacion(socket_servidor);

    if (respuesta == HANDSHAKE_OK)
    {
        log_info(cpu_logger, "Handshake con %s exitoso.", nombre_servidor);
        return true;
    }
    else
    {
        log_error(cpu_logger, "Falló el handshake con %s. Código recibido: %d", nombre_servidor, respuesta);
        return false;
    }
}