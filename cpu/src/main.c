#include "utils/protocolos.h"
#include "main.h"
#include "configuraciones.c"
#include "conexiones.c"
#include <pthread.h>
#include "instrucciones.c"
cpu_config  cpu_config;
t_log* cpuLogger = NULL; 

bool cpu_cargar_config(char* path)
{
    t_config* configuracion = config_create(path);
    if(!configuracion) 
    {
        return false;
    }

    cpu_config.IP_KERNEL_MEMORY    = strdup(config_get_string_value(configuracion, "IP_KERNEL_MEMORY"));
    cpu_config.PUERTO_KERNEL_MEMORY = strdup(config_get_string_value(configuracion, "PUERTO_KERNEL_MEMORY"));
    cpu_config.IP_KERNEL_SCHEDULER = strdup(config_get_string_value(configuracion, "IP_KERNEL_SCHEDULER"));
    cpu_config.PUERTO_KERNEL_SCHEDULER = strdup(config_get_string_value(configuracion, "PUERTO_KERNEL_SCHEDULER"));
    cpu_config.IP_MEMORY_STICK     = strdup(config_get_string_value(configuracion, "IP_MEMORY_STICK"));
    cpu_config.PUERTO_MEMORY_STICK = strdup(config_get_string_value(configuracion, "PUERTO_MEMORY_STICK"));

    const char* log_level_string = config_get_string_value(configuracion, "LOG_LEVEL");
    t_log_level level = log_level_from_string((char*)log_level_string);
    cpuLogger = log_create("cpu.log", "CPU", true, level);
    config_destroy(configuracion);
    return cpuLogger != NULL;
}
void cpu_liberar_config(void)
{   
    free(cpu_config.IP_KERNEL_MEMORY);
    free(cpu_config.IP_KERNEL_SCHEDULER);
    free(cpu_config.PUERTO_KERNER_MEMORY);
    free(cpu_config.PUERTO_KERNEL_SCHEDULER);
    free(cpu_config.PUERTO_MEMORY_STICK);
    close(cpu_config.SOCKET_KERNEL_MEMORY);
    close(cpu_config.SOCKET_KERNEL_SCHEDULER);
    close(cpu_config.SOCKET_MEMORY_STICK);
    free(cpu_config);
    if (cpuLogger) 
    {
        log_destroy(cpuLogger);
    }  
    return;
}


int main(int argc, char* argv[]) 
{
    if(!cpu_cargar_config("cpu.config")) 
    {
        fprintf(stderr, "Error al cargar la configuración\n");
        return EXIT_FAILURE;
    }
    if (conectar_a_kernel_memory() < 0) return EXIT_FAILURE; 
    if (conectar_a_kernel_scheduler() < 0) return EXIT_FAILURE;   
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, cpu_conexion_memoryStick, NULL);
    pthread_detach(hilo_cpu);

    cpu_liberar_config()

    return 0;
}
