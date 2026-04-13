#include "conexiones.h"


void realizar_handshake(int socket, int cod_op, char* nombre_modulo) {
    enviar_operacion(socket, cod_op);
    int respuesta = recibir_operacion(socket);
    if (respuesta != -1) {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
}
int conectar_a_kernel_memory(void)
{
    char *ipKernelMemory = cpuConfig.IP_KERNEL_MEMORY;
    char *puertoKernelMemory = cpuConfig.PUERTO_KERNEL_MEMORY;
    int socketKernelMemory = crear_conexion(ipKernelMemory, puertoKernelMemory);

    if (socketKernelMemory == -1 ) 
    {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Kernel Memory");
        config_destroy(cpuConfig);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_memory(cpuConfig, socketKernelMemory);

    enviar_operacion(socketKernelMemory, 1001);
    recibir_operacion(socketKernelMemory);
    char *nombre_modulo = "Kernel Memory";
    if (recibir_operacion(socketKernelMemory)!= -1) 
    {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
    else
    {
        log_error(cpuLogger, "Error al realizar handshake con %s", nombre_modulo);
        config_destroy(cpuConfig);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    return socketKernelMemory;
}

int conectar_a_kernel_scheduler(void)
{
    char *ipKernelScheduler = cpuConfig.IP_KERNEL_SCHEDULER;
    char *puertoKernelScheduler = cpuConfig.PUERTO_KERNEL_SCHEDULER;
    int socketKernelScheduler = crear_conexion(ipKernelScheduler, puertoKernelScheduler);

    if (socketKernelScheduler == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Kernel Scheduler");
        config_destroy(cpuConfig);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_scheduler(cpuConfig, socketKernelScheduler);

    enviar_operacion(socketKernelScheduler, 1001);
    recibir_operacion(socketKernelScheduler);
    char *nombre_modulo = "Kernel Scheduler";
    if (recibir_operacion(socketKernelScheduler)!= -1) 
    {
        log_info(cpuLogger, "Handshake con %s exitoso", nombre_modulo);
    }
    else
    {
        log_error(cpuLogger, "Error al realizar handshake con %s", nombre_modulo);
        config_destroy(cpuConfig);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    return socketKernelScheduler;
}

t_list* lista_sticks;

int conectar_a_memory_stick(void)
{
    char *ipMemoryStick = cpuConfig.IP_MEMORY_STICK;
    char *puertoMemoryStick = cpuConfig.PUERTO_MEMORY_STICK;
    int socketMemoryStick = crear_conexion(ipMemoryStick, puertoMemoryStick);

    if (socketMemoryStick == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        config_destroy(cpuConfig);
        log_destroy(cpuLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_memory_stick(cpuConfig, socketMemoryStick);

    enviar_operacion(socketMemoryStick, 1001);
    recibir_operacion(socketMemoryStick);

    return socketMemoryStick;
}
void conectar_a_nuevo_memory_stick(char* ip, char* puerto) {
    int nuevo_socket = crear_conexion(ip, puerto);
    if (nuevo_socket != -1) {
     
        enviar_operacion(nuevo_socket, 1001); 
        recibir_operacion(socketMemoryStick);
        list_add(lista_sticks, (void*)(intptr_t)nuevo_socket);
        log_info(cpuLogger, "Nuevo Memory Stick conectado en socket %d", nuevo_socket);
    }
}