#include <conexiones.h>


int conectar_a_kernel_memory(void)
{
    char *ipKernelMemory = cpuConfig.IP_KERNEL_MEMORY;
    char *puertoKernelMemory = cpuConfig.PUERTO_KERNEL_MEMORY;
    int socketKernelMemory = crear_conexion(ipKernelMemory, puertoKernelMemory);

    if (socketKernelMemory == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        log_error(cpuDebuggingLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        cpu_destroy(cpuConfig, cpuLogger, cpuDebuggingLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_memory(cpuConfig, socketKernelMemory);

    enviar_operacion(socketKernelMemory, 1001);
    recibir_operacion(socketKernelMemory);

    return socketKernelMemory;
}

int conectar_a_kernel_scheduler(void)
{
    char *ipKernelScheduler = cpuConfig.IP_KERNEL_SCHEDULER;
    char *puertoKernelScheduler = cpuConfig.PUERTO_KERNEL_SCHEDULER;
    int socketKernelScheduler = crear_conexion(ipKernelScheduler, puertoKernelScheduler);

    if (socketMemoria == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        log_error(cpuDebuggingLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        cpu_destroy(cpuConfig, cpuLogger, cpuDebuggingLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_scheduler(cpuConfig, socketKernelScheduler);

    enviar_operacion(socketKernelScheduler, 1001);
    recibir_operacion(socketKernelScheduler);

    return socketKernelScheduler;
}
int conectar_a_memory_stick(void)
{
    char *ipMemoryStick = cpuConfig.IP_MEMORY_STICK;
    char *puertoMemoryStick = cpuConfig.PUERTO_MEMORY_STICK;
    int socketMemoryStick = crear_conexion(ipMemoryStick, puertoMemoryStick);

    if (socketMemoria == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        log_error(cpuDebuggingLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        cpu_destroy(cpuConfig, cpuLogger, cpuDebuggingLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_memory_stick(cpuConfig, socketMemoryStick);

    enviar_operacion(socketMemoryStick, 1001);
    recibir_operacion(socketMemoryStick);

    return socketKernelScheduler;
}