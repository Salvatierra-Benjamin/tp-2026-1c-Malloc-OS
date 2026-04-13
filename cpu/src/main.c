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

void* cpu_conexion_memoryStick(void* _arg)
{
    (void)_arg;

    int socket_cpu = crear_conexion(cpu_config.IP_MEMORY_STICK,cpu_config.PUERTO_MEMORY_STICK);

    if(socket_cpu < 0)
    {
        log_error(cpuLogger, "No pude conectar al Memory Stick %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);
        return NULL;
    }

    log_info(cpuLogger, "Conectado a Memory Stick en %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);

    enviar_operacion(socket_cpu, 1001);

    int op = recibir_operacion(socket_cpu);

    if(op <= 0)
    {
        log_error(cpuLogger, "Memory Stick cerró durante handshake");
        close(socket_cpu);
        return NULL;
    }

    if(op != MS_OK_CONEXION){
        log_error(cpuLogger, "Handshake invalido. Esperaba MS_OK_CONEXION (%d), llegó %d", MS_OK_CONEXION, op);
        close(socket_cpu);
        return NULL;
    }

    log_info(cpuLogger, "Handshake con Memory Stick OK");

    // 🔁 Loop de comunicación con MS
    while(1){
        int cod_op = recibir_operacion(socket_cpu);

        if(cod_op <= 0){
            log_error(cpuLogger, "Memory Stick se desconectó");
            close(socket_cpu);
            break;
        }

        switch(cod_op){
            // case RESPUESTA_MEMORIA:
            //     ...
            //     break;

            default:
                log_warning(cpu_logger, "Operacion desconocida de MS (%d)", cod_op);
                t_paquete* basura = recibir_paquete(socket_cpu);
                if(basura) eliminar_paquete(basura);
                break;
        }
    }

    return NULL;
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
