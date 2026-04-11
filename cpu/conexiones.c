#include <conexiones.h>

int conectar_a_servidor(char *ip, char *puerto)
{
    int conexion;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    struct addrinfo *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rv = getaddrinfo(ip, puerto, &hints, &serverInfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo error: %s", gai_strerror(rv));
        return EXIT_FAILURE;
    }
    for (p = serverInfo; p != NULL; p = p->ai_next) {
        conexion = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (conn == -1) {
            continue;
        }
        // Si se conecto correctamente, salgo del bucle
        if (connect(conn, p->ai_addr, p->ai_addrlen) != -1) {
            break;
        }
        close(conexion);
    }

    freeaddrinfo(serverInfo);
    if (conexion != -1 && p != NULL) {
        return conn;
    }

    return -1;
}
int conectar_a_kernel_memory(void)
{
    char *ipKernelMemory = cpu_config_get_ip_kernel_memory(cpuConfig);
    char *puertoKernelMemory = cpu_config_get_puerto_kernel_memory(cpuConfig);
    int socketKernelMemory = conectar_a_servidor(ipMemoria, puertoKernelMemory);

    if (socketMemoria == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        log_error(cpuDebuggingLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        cpu_destroy(cpuConfig, cpuLogger, cpuDebuggingLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_memory(cpuConfig, socketKernelMemory);

    // Handshakes iniciales con módulo Memoria.
    enviar_operacion(socketKernelMemory, 1001);
    recibir_operacion(socketKernelMemory);

    return socketKernelMemory;
}

int conectar_a_kernel_scheduler(void)
{
    char *ipKernelScheduler = cpu_config_get_ip_kernel_scheduler(cpuConfig);
    char *puertoKernelScheduler = cpu_config_get_puerto_kernel_scheduler(cpuConfig);
    int socketKernelScheduler = conectar_a_servidor(ipMemoria, puertoMemoria);

    if (socketMemoria == -1 ) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        log_error(cpuDebuggingLogger, "Error al intentar establecer conexión inicial con el modulo Memoria");
        cpu_destroy(cpuConfig, cpuLogger, cpuDebuggingLogger);
        exit(EXIT_FAILURE);
    }

    cpu_config_set_socket_kernel_scheduler(cpuConfig, socketKernelMemory);

    // Handshakes iniciales con módulo Memoria.
    enviar_operacion(socketKernelScheduler, 1001);
    recibir_operacion(socketKernelScheduler);

    return socketKernelScheduler;
}
int conectar_a_memory_stick(void)
{

}