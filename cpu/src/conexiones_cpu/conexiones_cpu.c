#include "conexiones_cpu.h"
void creacion_conexiones_cpu()
{
    //! CONEXIÓN CPU->KERNEL_MEMORY
    conexion_km = crear_conexion(cpu_config.ip_kernel_memory, cpu_config.puerto_kernel_memory);
    if (conexion_km == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_km, HANDSHAKE_CPU); // HANDSHAKE CPU->KM

    //! ACLARACIÓN: CUANDO ESTE EN FUNCIONAMIENTO SE DESCOMENTA !!!!
    // //! CONEXIÓN CPU->KERNEL_SCHEDULER
    conexion_ks = crear_conexion(cpu_config.ip_kernel_scheduler, cpu_config.puerto_kernel_scheduler);
    if (conexion_ks == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KS.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_ks, HANDSHAKE_CPU); // HANDSHAKE CPU->KS

    //! CONEXIÓN CPU->MEMORY_STICK
    conexion_ms = crear_conexion(cpu_config.ip_memory_stick, cpu_config.puerto_memory_stick);
    if (conexion_ms == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->MS.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_ms, HANDSHAKE_CPU); // HANDSHAKE CPU->MS

    receptor_km();
    // receptor_ks();
    receptor_ms();
}
