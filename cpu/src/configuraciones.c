#include <configuraciones.h>
bool cpu_cargar_config(char* path)
{
    t_config* configuracion = config_create(path);
    if(!configuracion) 
    {
        return false;
    }

    cpu.puerto_escucha = strdup(config_get_string_value(configuracion,"PUERTO_ESCUCHA"));
    cpu.algoritmo      = strdup(config_get_string_value(configuracion,"ALGORITMO_PLANIFICACION")); // FIFO | PRIORIDADES
    cpu.tiempo_aging_ms= config_get_int_value(configuracion,"TIEMPO_AGING");
    const char* log_level_s = config_get_string_value(configuracion, "LOG_LEVEL");
    t_log_level lvl = log_level_from_string((char*)log_level_s);
    g_logger = log_create("master.log","MASTER",1, lvl);
    
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
    close(cpu_config.SOCKET_KERNEL_MEMORY);
    close(cpu_config.SOCKET_KERNEL_SCHEDULER);
    free(cpu_config);
    if (g_logger) {
        log_destroy(g_logger);
    }
    return;
}

uint32_t cpu_obtener_ip_kernel_memory(cpu_config config)
{
    return config.IP_KERNEL_MEMORY;
}
uint32_t cpu_obtener_ip_kernel_scheduler(cpu_config config)
{
    return config.IP_KERNEL_SCHEDULER;
}
uint32_t cpu_obtener_ip_memory_stick(cpu_config config)
{
    return config.IP_MEMORY_STICK;
}

int cpu_obtener_puerto_kernel_memory(cpu_config config)
{
    return config.PUERTO_KERNER_MEMORY;
}
int cpu_obtener_puerto_kernel_scheduler(cpu_config config)
{
    return config.PUERTO_KERNEL_SCHEDULER;
}
int cpu_obtener_puerto_memory_stick(cpu_config config)
{
    return config.PUERTO_MEMORY_STICK;
}
