#include "iniciador_cpu.h"
// Definiciones de Variables Globales y Funciones:
t_log *cpu_logger;
cpu_struct_config cpu_config;
int conexion_km;
int conexion_ks;
int identificador_cpu;
int PID_a_ejecutar = -1;
int PID_Interrupcion = -1;
t_list *lista_memory_sticks;
int flag_syscall = -1;
int flag_seg_fault = 0;
int TAM_MAX_SEGMENTO = -1;
t_contexto_ejecucion contexto_aux_global;
t_instruccion instruccion_aux_global;
//  Semaforos:
sem_t sem_proceso_nuevo;
sem_t sem_contexto_recibido;
sem_t sem_contexto_procesado;
sem_t sem_instruccion_recibida;
sem_t sem_instruccion_procesada;
sem_t hay_interrupcion;
sem_t semaphore;                                                // Coordinacion //POR AHORA NO SE USA
pthread_mutex_t mutex_lista_sticks = PTHREAD_MUTEX_INITIALIZER; // MUTEX
pthread_mutex_t conexiones_km = PTHREAD_MUTEX_INITIALIZER;      // MUTEX
pthread_mutex_t conexiones_ks = PTHREAD_MUTEX_INITIALIZER;      // MUTEX

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
    cpu_config.puerto_kernel_memory = strdup(config_get_string_value(config, "PUERTO_KERNEL_MEMORY"));
    cpu_config.puerto_kernel_scheduler = strdup(config_get_string_value(config, "PUERTO_KERNEL_SCHEDULER"));

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

    // Puertos
    log_debug(cpu_logger, "Puerto Kernel Memory: %s", cpu_config.puerto_kernel_memory);
    log_debug(cpu_logger, "Puerto Kernel Scheduler: %s", cpu_config.puerto_kernel_scheduler);

    // Log Level
    log_debug(cpu_logger, "Nivel de Log: %d\n", cpu_config.log_level);
}

void inicializar_semaforos_coordinadores()
{
    // 0: El semáforo se comparte entre hilos del mismo proceso
    // 0: Valor inicial (el semáforo empieza "cerrado")
    sem_init(&sem_proceso_nuevo, 0, 0);
    sem_init(&sem_contexto_recibido, 0, 0);
    sem_init(&sem_contexto_procesado, 0, 0);
    sem_init(&sem_instruccion_recibida, 0, 0);
    sem_init(&sem_instruccion_procesada, 0, 0);
    sem_init(&hay_interrupcion, 0, 0);
    sem_init(&semaphore, 0, 0);
}

void cerrar_conexiones()
{
    close(conexion_ks);
    close(conexion_km);
}

void liberar_semaforos()
{
    pthread_mutex_destroy(&mutex_lista_sticks);
    pthread_mutex_destroy(&conexiones_km);
    pthread_mutex_destroy(&conexiones_ks);
    sem_destroy(&sem_proceso_nuevo);
    sem_destroy(&sem_contexto_recibido);
    sem_destroy(&sem_contexto_procesado);
    sem_destroy(&sem_instruccion_recibida);
    sem_destroy(&sem_instruccion_procesada);
    sem_destroy(&hay_interrupcion);
    sem_destroy(&semaphore);
}

void cpu_liberar_config()
{
    free(cpu_config.ip_kernel_memory);
    free(cpu_config.ip_kernel_scheduler);
    free(cpu_config.puerto_kernel_memory);
    free(cpu_config.puerto_kernel_scheduler);

    log_debug(cpu_logger, "Estructura de la Config Liberada\n");
}

void cpu_liberar_log()
{
    log_debug(cpu_logger, "Log Liberado!!!\n");
    log_destroy(cpu_logger);
}

void destruir_memory_stick(void *element)
{
    t_memory_stick *ms = (t_memory_stick *)element;

    // 1. Cerrar la conexión (socket)
    if (ms->socket != -1)
    {
        close(ms->socket);
    }

    // 2. Liberar los campos dinámicos
    free(ms->ip);
    free(ms->puerto);

    // 3. Liberar la estructura en sí
    free(ms);
}

void liberar_todo()
{
    cerrar_conexiones();
    liberar_semaforos();
    cpu_liberar_config();
    cpu_liberar_log();
    list_destroy_and_destroy_elements(lista_memory_sticks, destruir_memory_stick);
}