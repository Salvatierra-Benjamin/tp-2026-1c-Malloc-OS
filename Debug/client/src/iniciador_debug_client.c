#include "iniciador_debug_client.h"
// Definiciones de Variables Globales y Funciones:
t_log *debug_client_logger;
debug_client_struct_config debug_client_config;
int conexion_debug_server;
int identificador_debug_client;
sem_t sem_proceso_nuevo;
int PID_a_ejecutar = -1;

void debug_client_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE debug_client\n");
        exit(EXIT_FAILURE);
    }
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    debug_client_config.log_level = log_level_from_string((char *)log_level_string);
    debug_client_config.ip_debug_server = strdup(config_get_string_value(config, "IP_DEBUG_SERVER"));
    debug_client_config.puerto_debug_server = strdup(config_get_string_value(config, "PUERTO_DEBUG_SERVER"));

    config_destroy(config);
}

void inicializar_logger()
{

    char *ruta_con_debug_client_id = string_from_format("debug_client_%d_.log", identificador_debug_client);

    debug_client_logger = log_create(ruta_con_debug_client_id, "debug_client", 1, debug_client_config.log_level);

    if (debug_client_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER debug_client\n");
        exit(EXIT_FAILURE);
    }
    log_debug(debug_client_logger, "Se creo log de debug_client correctamente");
    free(ruta_con_debug_client_id);
}

void imprimir_valores_config_debug_client()
{
    log_debug(debug_client_logger, "Valores cargados en debug_client:");

    // IPs
    log_debug(debug_client_logger, "IP_DEBUG_SERVER: %s", debug_client_config.ip_debug_server);

    // Puertos
    log_debug(debug_client_logger, "PUERTO_DEBUG_SERVER: %s", debug_client_config.puerto_debug_server);

    // Log Level
    log_debug(debug_client_logger, "Nivel de Log: %d\n", debug_client_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    // 0: El semáforo se comparte entre hilos del mismo proceso
    // 0: Valor inicial (el semáforo empieza "cerrado")
    sem_init(&sem_proceso_nuevo, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_debug_server);
}

void liberar_semaforos()
{
    sem_destroy(&sem_proceso_nuevo);
}

void debug_client_liberar_config()
{
    free(debug_client_config.ip_debug_server);
    free(debug_client_config.puerto_debug_server);

    log_debug(debug_client_logger, "Estructura de la Config Liberada\n");
}

void debug_client_liberar_log()
{
    log_debug(debug_client_logger, "Log Liberado!!!\n");
    log_destroy(debug_client_logger);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    debug_client_liberar_config();
    debug_client_liberar_log();
}