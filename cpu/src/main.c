#include "utils/hello.h"
#include "cpu.h"
#include "configuraciones.c"
t_log* cpu_logger = NULL;
cpu_config  cpu_config;
t_log_level LOG_LEVEL_INFO = log_level_from_string(cpuConfig.LOG_LEVEL);
bool cpu_cargar_config(char* path)
{
    t_config* configuracion = config_create(path);
    if(!configuracion) 
    {
        return false;
    }

    cpu.ip_kernel_memory    = strdup(config_get_string_value(configuracion, "IP_KERNEL_MEMORY"));
    cpu.puerto_kernel_memory = strdup(config_get_string_value(configuracion, "PUERTO_KERNEL_MEMORY"));
    cpu.ip_kernel_scheduler = strdup(config_get_string_value(configuracion, "IP_KERNEL_SCHEDULER"));
    cpu.puerto_kernel_scheduler = strdup(config_get_string_value(configuracion, "PUERTO_KERNEL_SCHEDULER"));
    cpu.ip_memory_stick     = strdup(config_get_string_value(configuracion, "IP_MEMORY_STICK"));
    cpu.puerto_memory_stick = strdup(config_get_string_value(configuracion, "PUERTO_MEMORY_STICK"));

    const char* log_level_s = config_get_string_value(configuracion, "LOG_LEVEL");
    t_log_level lvl = log_level_from_string((char*)log_level_s);
    char* nombre_archivo_log = string_from_format("cpu_%s.log", id_cpu); 
    g_logger = log_create(nombre_archivo_log, "CPU", 1, lvl); [cite: 162]
    free(nombre_archivo_log); 
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
    close(cpu_config.SOCKET_MEMORY_STICK);
    free(cpu_config);
    if (g_logger) {
        log_destroy(g_logger);
    }
    return;
}
void* cpu_conexion_memoryStick(void* _arg)
{
    (void)_arg;

    int socket_cpu = crear_conexion(cpu_config.IP_MEMORY_STICK,cpu_config.PUERTO_MEMORY_STICK);

    if(socket_cpu < 0)
    {
        log_error(ks_logger, "No pude conectar al Memory Stick %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);
        return NULL;
    }

    log_info(cpu_logger, "Conectado a Kernel Memory en %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);

    enviar_operacion(socket_cpu, 1001);

    int op = recibir_operacion(socket_cpu);

    if(op <= 0)
    {
        log_error(ks_logger, "Memory Stick cerró durante handshake");
        close(socket_cpu);
        return NULL;
    }

    if(op != MS_OK_CONEXION){
        log_error(cpu_logger, "Handshake invalido. Esperaba MS_OK_CONEXION (%d), llegó %d", MS_OK_CONEXION, op);
        close(socket_cpu);
        return NULL;
    }

    log_info(cpu_logger, "Handshake con Memory Stick OK");

    // 🔁 Loop de comunicación con KM
    while(1){
        int cod_op = recibir_operacion(socket_cpu);

        if(cod_op <= 0){
            log_error(cpu_logger, "Memory Stick se desconectó");
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
    if (conectar_a_kernel_memory() < 0) return EXIT_FAILURE; [cite: 163]
    if (conectar_a_kernel_scheduler() < 0) return EXIT_FAILURE; [cite: 163]
    if (conectar_a_memory_stick() < 0) 
    {
        log_warning(g_logger, "No se encontraron Memory Sticks iniciales, esperando conexión dinámica...");
    }
    
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, cpu_conexion_memoryStick, NULL);
    pthread_detach(hilo_cpu);

    cpu_liberar_config()

    return 0;
}
