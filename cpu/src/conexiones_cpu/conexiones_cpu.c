#include "conexiones_cpu.h"

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
    receptor_km();
}