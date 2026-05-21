#include "conexiones_ms.h"
void creacion_conexiones_ms()
{
    conexion_km = crear_conexion(ms_config.ip_km, ms_config.puerto_km);
    if (conexion_km == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION MS->KM.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    enviar_operacion(conexion_km, HANDSHAKE_MS);

    receptor_km();

    socket_servidor_con_cpu = iniciar_servidor(ms_config.puerto_escucha_cpu);
    if (socket_servidor_con_cpu == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION CPU->MS.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    esperar_cpus();
}
