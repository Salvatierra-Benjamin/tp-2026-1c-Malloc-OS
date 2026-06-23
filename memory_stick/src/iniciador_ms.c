#include "iniciador_ms.h"
// Definiciones de Variables Globales y Funciones:
t_log *ms_logger;
ms_config_struct ms_config;
int conexion_km;
int socket_servidor_con_cpu;
int tamanio_memoria_ms_ingresada;
void *puntero_memoria_ms;
// Semaforos:
pthread_mutex_t conexiones_km = PTHREAD_MUTEX_INITIALIZER;   // MUTEX
pthread_mutex_t sockets_cliente = PTHREAD_MUTEX_INITIALIZER; // MUTEX
sem_t semaphore;
sem_t sem_ok_conexion_km;

void ms_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE MEMORY STICK\n");
        exit(EXIT_FAILURE);
    }
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    ms_config.log_level = log_level_from_string((char *)log_level_string);
    ms_config.memory_delay = config_get_int_value(config, "MEMORY_DELAY");
    ms_config.ip_ms = strdup(config_get_string_value(config, "IP_MEMORY_STICK"));
    ms_config.puerto_escucha_cpu = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_CPU"));
    ms_config.ip_km = strdup(config_get_string_value(config, "IP_KM"));
    ms_config.puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
    ms_config.mi_ip = strdup(config_get_string_value(config, "MI_IP"));
    
    config_destroy(config);
}

void inicializar_logger()
{
    ms_logger = log_create("memory_stick.log", "memory_stick", 1, ms_config.log_level);

    if (ms_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER MEMORY STICK\n");
        exit(EXIT_FAILURE);
    }
    log_debug(ms_logger, "Se creo log de MEMORY STICK correctamente");
}

void imprimir_valores_config_ms()
{
    log_debug(ms_logger, "Valores cargados en MEMORY STICK:");
    log_debug(ms_logger, "Puerto Escucha CPU: %s", ms_config.puerto_escucha_cpu);
    log_debug(ms_logger, "Memory Delay: %d", ms_config.memory_delay);
    log_debug(ms_logger, "IP KM: %s", ms_config.ip_km);
    log_debug(ms_logger, "Puerto KM: %s", ms_config.puerto_km);
    log_debug(ms_logger, "Nivel de Log: %d\n", ms_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    // 0: El semáforo se comparte entre hilos del mismo proceso
    // 0: Valor inicial (el semáforo empieza "cerrado")
    sem_init(&semaphore, 0, 0);
    sem_init(&sem_ok_conexion_km, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_km);
}

void liberar_semaforos()
{
    pthread_mutex_destroy(&conexiones_km);
    sem_destroy(&semaphore);
    sem_destroy(&sem_ok_conexion_km);
}

void ms_liberar_config()
{
    free(ms_config.ip_ms);
    free(ms_config.puerto_escucha_cpu);
    free(ms_config.ip_km);
    free(ms_config.puerto_km);

    log_debug(ms_logger, "Estructura de la Config Liberada\n");
}

void ms_liberar_log()
{
    log_debug(ms_logger, "Log Liberado!!!\n");
    log_destroy(ms_logger);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    ms_liberar_config();
    ms_liberar_log();
    free(puntero_memoria_ms);
}