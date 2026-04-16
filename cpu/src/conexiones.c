#include "conexiones.h"

void realizar_handshake(int socket, int cod_op, char *nombre_modulo)
{
    enviar_operacion(socket, cod_op);
    int respuesta = recibir_operacion(socket);
    if (respuesta != -1)
    {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
}
int conectar_a_kernel_memory(void)
{
    char *ipKernelMemory = cpu_config.IP_KERNEL_MEMORY;
    char *puertoKernelMemory = cpu_config.PUERTO_KERNEL_MEMORY;
    int socketKernelMemory = crear_conexion(ipKernelMemory, puertoKernelMemory);

    if (socketKernelMemory == -1)
    {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Kernel Memory");
        // config_destroy(cpu_config);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_memory(socketKernelMemory);

    enviar_operacion(socketKernelMemory, 1001);
    recibir_operacion(socketKernelMemory);
    char *nombre_modulo = "Kernel Memory";
    if (recibir_operacion(socketKernelMemory) != -1)
    {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
    else
    {
        log_error(cpuLogger, "Error al realizar handshake con %s", nombre_modulo);
        // config_destroy(cpu_config);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    return socketKernelMemory;
}

int conectar_a_kernel_scheduler(void)
{
    char *ipKernelScheduler = cpu_config.IP_KERNEL_SCHEDULER;
    char *puertoKernelScheduler = cpu_config.PUERTO_KERNEL_SCHEDULER;
    int socketKernelScheduler = crear_conexion(ipKernelScheduler, puertoKernelScheduler);

    if (socketKernelScheduler == -1)
    {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Kernel Scheduler");
        // config_destroy(cpu_config);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_scheduler(socketKernelScheduler);

    enviar_operacion(socketKernelScheduler, 1001);
    recibir_operacion(socketKernelScheduler);
    char *nombre_modulo = "Kernel Scheduler";
    if (recibir_operacion(socketKernelScheduler) != -1)
    {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
    else
    {
        log_error(cpuLogger, "Error al realizar handshake con %s", nombre_modulo);
        // config_destroy(cpu_config);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    return socketKernelScheduler;
}

void *cpu_conexion_memoryStick(void *_arg)
{
    (void)_arg;

    int socket_cpu = crear_conexion(cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);

    if (socket_cpu < 0)
    {
        log_error(cpuLogger, "No pude conectar al Memory Stick %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);
        return NULL;
    }

    log_info(cpuLogger, "Conectado a Memory Stick en %s:%s", cpu_config.IP_MEMORY_STICK, cpu_config.PUERTO_MEMORY_STICK);

    enviar_operacion(socket_cpu, HANDSHAKE_CPU);

    int op = recibir_operacion(socket_cpu);

    if (op <= 0)
    {
        log_error(cpuLogger, "Memory Stick cerró durante handshake");
        close(socket_cpu);
        return NULL;
    }

    if (op != MS_OK_CONEXION)
    {
        log_error(cpuLogger, "Handshake invalido. Esperaba MS_OK_CONEXION (%d), llegó %d", MS_OK_CONEXION, op);
        close(socket_cpu);
        return NULL;
    }

    log_info(cpuLogger, "Handshake con Memory Stick OK");

    // 🔁 Loop de comunicación con MS
    while (1)
    {
        int cod_op = recibir_operacion(socket_cpu);

        if (cod_op <= 0)
        {
            log_error(cpuLogger, "Memory Stick se desconectó");
            close(socket_cpu);
            break;
        }

        switch (cod_op)
        {
            // case RESPUESTA_MEMORIA:
            //     ...
            //     break;

        default:
            log_warning(cpuLogger, "Operacion desconocida de MS (%d)", cod_op);
            t_paquete *basura = recibir_paquete(socket_cpu);
            if (basura)
                eliminar_paquete(basura);
            break;
        }
    }

    return NULL;
}