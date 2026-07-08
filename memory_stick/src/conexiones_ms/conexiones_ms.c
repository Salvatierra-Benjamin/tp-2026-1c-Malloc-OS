#include "conexiones_ms.h"
void creacion_conexiones_ms()
{
    // El puerto de escucha (CPUs + conexión de datos de KM) debe estar en listen ANTES
    // de registrarnos en KM. Apenas KM recibe nuestro MEMORY_STICK_SIZE abre su conexión
    // de datos contra este puerto; si todavía no hicimos bind+listen, ese connect recibe
    // ECONNREFUSED y KM registra un stick sin conexión de datos (R/W y suspensión rotas).
    // iniciar_servidor solo hace bind+listen (no bloquea); el accept vive en esperar_cpus,
    // así que el backlog retiene el connect de KM hasta que lo aceptemos.
    socket_servidor_con_cpu = iniciar_servidor(ms_config.puerto_escucha_cpu);
    if (socket_servidor_con_cpu == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION CPU->MS.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    conexion_km = crear_conexion(ms_config.ip_km, ms_config.puerto_km);
    if (conexion_km == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION MS->KM.\n");
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    enviar_operacion(conexion_km, HANDSHAKE_MS);
    receptor_km();
    sem_wait(&sem_ok_conexion_km);
    esperar_cpus();
}
