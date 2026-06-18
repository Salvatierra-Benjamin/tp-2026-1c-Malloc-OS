#include "iniciador_cpu.h"
// Definiciones de Variables Globales y Funciones:
t_log *cpu_logger;
cpu_struct_config cpu_config;
int conexion_km;
int conexion_ks;
int conexion_ms;
int identificador_cpu;
sem_t sem_proceso_nuevo;
sem_t hay_interrupcion;
int PID_a_ejecutar = -1;
int PID_Interrupcion = -1;

//  Semaforos:
pthread_mutex_t conexiones_km = PTHREAD_MUTEX_INITIALIZER; // MUTEX
pthread_mutex_t conexiones_ks = PTHREAD_MUTEX_INITIALIZER; // MUTEX
pthread_mutex_t conexiones_ms = PTHREAD_MUTEX_INITIALIZER; // MUTEX
sem_t semaphore;                                           // Coordinacion //POR AHORA NO SE USA

void cpu_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE CPU\n");
        exit(EXIT_FAILURE);
    }
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    cpu_config.log_level = log_level_from_string((char *)log_level_string);
    cpu_config.ip_kernel_memory = strdup(config_get_string_value(config, "IP_KERNEL_MEMORY"));
    cpu_config.ip_kernel_scheduler = strdup(config_get_string_value(config, "IP_KERNEL_SCHEDULER"));
    cpu_config.ip_memory_stick = strdup(config_get_string_value(config, "IP_MEMORY_STICK"));
    cpu_config.puerto_kernel_memory = strdup(config_get_string_value(config, "PUERTO_KERNEL_MEMORY"));
    cpu_config.puerto_kernel_scheduler = strdup(config_get_string_value(config, "PUERTO_KERNEL_SCHEDULER"));
    cpu_config.puerto_memory_stick = strdup(config_get_string_value(config, "PUERTO_MEMORY_STICK"));

    config_destroy(config);
}

void inicializar_logger()
{

    char *ruta_con_cpu_id = string_from_format("cpu_%d_.log", identificador_cpu);

    cpu_logger = log_create(ruta_con_cpu_id, "CPU", 1, cpu_config.log_level);

    if (cpu_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER CPU\n");
        exit(EXIT_FAILURE);
    }
    log_debug(cpu_logger, "Se creo log de CPU correctamente");
    free(ruta_con_cpu_id);
}

void imprimir_valores_config_cpu()
{
    log_debug(cpu_logger, "Valores cargados en CPU:");

    // IPs
    log_debug(cpu_logger, "IP Kernel Memory: %s", cpu_config.ip_kernel_memory);
    log_debug(cpu_logger, "IP Kernel Scheduler: %s", cpu_config.ip_kernel_scheduler);
    log_debug(cpu_logger, "IP Memory Stick: %s", cpu_config.ip_memory_stick);

    // Puertos
    log_debug(cpu_logger, "Puerto Kernel Memory: %s", cpu_config.puerto_kernel_memory);
    log_debug(cpu_logger, "Puerto Kernel Scheduler: %s", cpu_config.puerto_kernel_scheduler);
    log_debug(cpu_logger, "Puerto Memory Stick: %s", cpu_config.puerto_memory_stick);

    // Log Level
    log_debug(cpu_logger, "Nivel de Log: %d\n", cpu_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    // 0: El semáforo se comparte entre hilos del mismo proceso
    // 0: Valor inicial (el semáforo empieza "cerrado")
    sem_init(&sem_proceso_nuevo, 0, 0);
    sem_init(&hay_interrupcion, 0, 0);
    sem_init(&semaphore, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_ks);
    close(conexion_km);
    close(conexion_ms);
}

void liberar_semaforos()
{
    pthread_mutex_destroy(&conexiones_km);
    pthread_mutex_destroy(&conexiones_ks);
    pthread_mutex_destroy(&conexiones_ms);
    sem_destroy(&sem_proceso_nuevo);
    sem_destroy(&hay_interrupcion);
    sem_destroy(&semaphore);
}

void cpu_liberar_config()
{
    free(cpu_config.ip_kernel_memory);
    free(cpu_config.ip_kernel_scheduler);
    free(cpu_config.ip_memory_stick);
    free(cpu_config.puerto_kernel_memory);
    free(cpu_config.puerto_kernel_scheduler);
    free(cpu_config.puerto_memory_stick);

    log_debug(cpu_logger, "Estructura de la Config Liberada\n");
}

void cpu_liberar_log()
{
    log_debug(cpu_logger, "Log Liberado!!!\n");
    log_destroy(cpu_logger);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    cpu_liberar_config();
    cpu_liberar_log();
}