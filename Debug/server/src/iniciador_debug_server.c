#include "iniciador_debug_server.h"
// Definiciones de Variables Globales y Funciones:
t_log *debug_server_logger;
debug_server_config_struct debug_server_config;
int conexion_debug_server;
int socket_servidor_con_debug_client;
int tamanio_memoria;
void *memoria_debug_server;
sem_t semaphore;
int PID_a_ejecutar = -1;

void debug_server_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE debug_server\n");
        exit(EXIT_FAILURE);
    }
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    debug_server_config.log_level = log_level_from_string((char *)log_level_string);
    debug_server_config.puerto_escucha_debug_client = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_DEBUG_CLIENT"));

    config_destroy(config);
}

void inicializar_logger()
{
    debug_server_logger = log_create("debug_server.log", "debug_server", 1, debug_server_config.log_level);

    if (debug_server_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER debug_server\n");
        exit(EXIT_FAILURE);
    }
    log_debug(debug_server_logger, "Se creo log de debug_server correctamente");
}

void imprimir_valores_config_debug_server()
{
    log_debug(debug_server_logger, "Valores cargados en debug_server:");
    log_debug(debug_server_logger, "PUERTO_ESCUCHA_DEBUG_CLIENT: %s", debug_server_config.puerto_escucha_debug_client);
    log_debug(debug_server_logger, "Nivel de Log: %d\n", debug_server_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    sem_init(&semaphore, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_debug_server);
}

void liberar_semaforos()
{
    sem_destroy(&semaphore);
}

void debug_server_liberar_config()
{
    free(debug_server_config.puerto_escucha_debug_client);

    log_debug(debug_server_logger, "Estructura de la Config Liberada\n");
}

void debug_server_liberar_log()
{
    log_debug(debug_server_logger, "Log Liberado!!!\n");
    log_destroy(debug_server_logger);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    debug_server_liberar_config();
    debug_server_liberar_log();
}