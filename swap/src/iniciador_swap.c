#include "iniciador_swap.h"
// Definiciones de Variables Globales y Funciones:
t_log *swap_logger;
swap_config_struct swap_config;
int conexion_km;
FILE *archivo_swap;
// Semaforos:
pthread_mutex_t mutex_escritura = PTHREAD_MUTEX_INITIALIZER; // MUTEX
pthread_mutex_t mutex_lecutra = PTHREAD_MUTEX_INITIALIZER;   // MUTEX
pthread_mutex_t conexiones_km = PTHREAD_MUTEX_INITIALIZER;   // MUTEX
sem_t semaphore;

void swap_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE SWAP\n");
        exit(EXIT_FAILURE);
    }
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    swap_config.log_level = log_level_from_string((char *)log_level_string);
    swap_config.swap_file_path = strdup(config_get_string_value(config, "SWAP_FILE_PATH"));
    swap_config.swap_file_size = config_get_int_value(config, "SWAP_FILE_SIZE");
    swap_config.swap_block_size = config_get_int_value(config, "BLOCK_SIZE");
    swap_config.ip_km = strdup(config_get_string_value(config, "IP_KM"));
    swap_config.puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));

    config_destroy(config);
}

void inicializar_logger()
{
    swap_logger = log_create("swap.log", "SWAP", 1, swap_config.log_level);

    if (swap_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER SWAP\n");
        exit(EXIT_FAILURE);
    }
    log_debug(swap_logger, "Se creo log de Swap correctamente");
}

void imprimir_valores_config_swap()
{
    log_debug(swap_logger, "Valores cargados en SWAP:");
    log_debug(swap_logger, "Path Archivo: %s", swap_config.swap_file_path);
    log_debug(swap_logger, "Tamaño Archivo: %d", swap_config.swap_file_size);
    log_debug(swap_logger, "Tamaño Bloque: %d", swap_config.swap_block_size);
    log_debug(swap_logger, "IP KM: %s", swap_config.ip_km);
    log_debug(swap_logger, "Puerto KM: %s", swap_config.puerto_km);
    log_debug(swap_logger, "Nivel de Log: %d\n", swap_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    // 0: El semáforo se comparte entre hilos del mismo proceso
    // 0: Valor inicial (el semáforo empieza "cerrado")
    sem_init(&semaphore, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_km);
}

void liberar_semaforos()
{
    pthread_mutex_destroy(&mutex_escritura);
    pthread_mutex_destroy(&mutex_lecutra);
    pthread_mutex_destroy(&conexiones_km);
    sem_destroy(&semaphore);
}

void swap_liberar_config()
{
    free(swap_config.swap_file_path);
    free(swap_config.ip_km);
    free(swap_config.puerto_km);

    log_debug(swap_logger, "Estructura de la Config Liberada\n");
}

void swap_liberar_log()
{
    log_debug(swap_logger, "Log Liberado!!!\n");
    log_destroy(swap_logger);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    swap_liberar_config();
    swap_liberar_log();
}