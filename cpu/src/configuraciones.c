#include "configuraciones.h"
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
    const char* log_level_s = config_get_string_value(configuracion, "LOG_LEVEL");
    cpu_config.LOG_LEVEL = strdup(log_level_s);
    t_log_level LOG_LEVEL = log_level_from_string((char*)log_level_s);
    g_logger = log_create("cpu.log","cpu",1, LOG_LEVEL);
    
    config_destroy(configuracion);
    return g_logger != NULL;
}
void cpu_liberar_config(void)
{   
    free(cpu_config.IP_KERNEL_MEMORY);
    free(cpu_config.IP_KERNEL_SCHEDULER);
    free(cpu_config.PUERTO_KERNER_MEMORY);
    free(cpu_config.PUERTO_KERNEL_SCHEDULER);
    free(cpu_config.PUERTO_MEMORY_STICK);
    free(cpu_config.LOG_LEVEL);
    close(cpu_config.SOCKET_KERNEL_MEMORY);
    close(cpu_config.SOCKET_KERNEL_SCHEDULER);
    close(cpu_config.SOCKET_MEMORY_STICK);
    free(cpu_config);
    if (g_logger) {
        log_destroy(g_logger);
    }
    return;
}

void cpu_config_set_socket_kernel_scheduler(cpu_config* config, int socket)
{
     config->SOCKET_KERNEL_SCHEDULER = socket;
}
void cpu_config_set_socket_kernel_memory(cpu_config *config, int socket)
{
    config->SOCKET_KERNEL_MEMORY = socket;
}
void cpu_config_set_socket_memory_stick(cpu_config *config, int socket)
{
    config->SOCKET_MEMORY_STICK = socket;
}


