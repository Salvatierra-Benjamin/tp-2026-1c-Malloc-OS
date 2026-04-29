#include "conexiones_cpu.h"
void creacion_conexiones_cpu()
{

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
    // enviar_operacion(conexion_ks, HANDSHAKE_CPU); // HANDSHAKE CPU->KS //! Se cambia por un handshake con enviar paquete con el id
    // enviar_paquete con el id y HANDSHAKE_CPU
    t_paquete * paquete_handshake_ks = crear_paquete(HANDSHAKE_CPU);
    agregar_a_paquete(paquete_handshake_ks, &identificador,sizeof(int));
    enviar_paquete(paquete_handshake_ks,conexion_ks);
    eliminar_paquete(paquete_handshake_ks);


    //! CONEXIÓN CPU->KERNEL_MEMORY
    conexion_km = crear_conexion(cpu_config.ip_kernel_memory, cpu_config.puerto_kernel_memory);
    if (conexion_km == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        exit(EXIT_FAILURE);
    }
    // enviar_operacion(conexion_km, HANDSHAKE_CPU); // HANDSHAKE CPU->KM //! Se cambia por un handshake con enviar paquete con el id
    // enviar_paquete con el id y HANDSHAKE_CPU
    t_paquete * paquete_handshake_km = crear_paquete(HANDSHAKE_CPU);
    agregar_a_paquete(paquete_handshake_km, &identificador,sizeof(int));
    enviar_paquete(paquete_handshake_km,conexion_km);
    eliminar_paquete(paquete_handshake_km);

    //! CONEXIÓN CPU->MEMORY_STICK
    conexion_ms = crear_conexion(cpu_config.ip_memory_stick, cpu_config.puerto_memory_stick);
    if (conexion_ms == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->MS.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        exit(EXIT_FAILURE);
    }
    // enviar_operacion(conexion_ms, HANDSHAKE_CPU); // HANDSHAKE CPU->MS //! Se cambia por un handshake con enviar paquete con el id
    // enviar_paquete con el id y HANDSHAKE_CPU

    t_paquete * paquete_handshake_ms = crear_paquete(HANDSHAKE_CPU);
    agregar_a_paquete(paquete_handshake_ms, &identificador,sizeof(int));
    enviar_paquete(paquete_handshake_ms,conexion_ms);
    eliminar_paquete(paquete_handshake_ms);

    // receptor_ks();
    receptor_km();
    receptor_ms();
}
