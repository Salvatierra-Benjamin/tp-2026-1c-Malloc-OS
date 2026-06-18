#include "conexiones_cpu.h"

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

void creacion_conexiones_cpu()
{
    // ACLARACIÓN: RECEPTOR_KS SOLO USA LA CONEXION DEL KS. POR OTRO LADO, EN EL MAIN SE USAN LAS DEL KM Y MS.
    // NO SE RECOMIENDA UTILIZARLAS EN OTROS .C, GENERARIA POSIBLE CONDICION DE CARRERA.
    // PARA SINCRONIZAR LOS PROCESOS SE RECOMIENDA EL USO DE SEMAFOROS CONTADORES.

    // --- CONEXIÓN AL KERNEL SCHEDULER ---
    conexion_ks = crear_conexion(cpu_config.ip_kernel_scheduler, cpu_config.puerto_kernel_scheduler);
    if (conexion_ks == -1)
    {
        log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->KS.");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    if (!realizar_handshake_ks(conexion_ks, identificador_cpu, "Kernel Scheduler"))
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

    // --- CONEXIÓN AL MEMORY STICK ---
    conexion_ms = crear_conexion(cpu_config.ip_memory_stick, cpu_config.puerto_memory_stick);
    if (conexion_ms == -1)
    {
        log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    if (!realizar_handshake(conexion_ms, identificador_cpu, "Kernel Memory"))
    {
        liberar_todo();
        exit(EXIT_FAILURE);
    }
}