#include "conexiones_cpu.h"

// Retorna true si el servidor aceptó la conexión, false si hubo un error
bool realizar_handshake(int socket_servidor, int id_cpu, char *nombre_servidor)
{
    // 1. Enviamos nuestro ID de CPU
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

void creacion_conexiones_cpu()
{

    // --- CONEXIÓN AL KERNEL SCHEDULER ---
    conexion_ks = crear_conexion(cpu_config.ip_kernel_scheduler, cpu_config.puerto_kernel_scheduler);
    if (conexion_ks == -1)
    {
        log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->KS.");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    if (!realizar_handshake(conexion_ks, identificador_cpu, "Kernel Scheduler"))
    {
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    receptor_ks();

    // --- CONEXIÓN AL KERNEL MEMORY ---
    conexion_km = crear_conexion(cpu_config.ip_kernel_memory, cpu_config.puerto_kernel_memory);
    if (conexion_km == -1)
    {
        log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    if (!realizar_handshake(conexion_km, identificador_cpu, "Kernel Memory"))
    {
        liberar_todo();
        exit(EXIT_FAILURE);
    }
}