#include "atender_cpu.h"
void esperar_cpus(int socket_servidor)
{
    while (1)
    {
        log_debug(ms_logger, "Esperando cpus...");
        int socket_cliente = esperar_cliente(socket_servidor);
        int *socket_cliente_puntero = malloc(sizeof(int));
        *socket_cliente_puntero = socket_cliente;
        pthread_t hilo_atencion_cpus;
        pthread_create(&hilo_atencion_cpus, NULL, atencion_cpus, (void *)socket_cliente_puntero);
        pthread_detach(hilo_atencion_cpus); // Cuando este hilo termine, que nadie lo espere; libera su memoria automáticamente.
        // Si fuera pthread_join (hilo_atencion_cpus); dejaria de ser multihilo ya que esta funcion es bloqueante entonces la CPU 2 debe esperar a que termine la CPU 1.
    }
}

void *atencion_cpus(void *socket_cliente_puntero)
{
    log_debug(ms_logger, "Se conecto una cpu");
    int socket_cliente = *((int *)(socket_cliente_puntero));
    free(socket_cliente_puntero);

    while (1)
    {
        int cod_op = recibir_operacion(socket_cliente);

        switch (cod_op)
        {
        case HANDSHAKE_CPU:
        {
            enviar_operacion(socket_cliente, CPU_OK_CONEXION);
            break;
        }
        case -1:
        {
            log_error(ms_logger, "Error de comunicación con la cpu");
            close(socket_cliente);
            pthread_exit(NULL);
        }
        default:
        {
            log_warning(ms_logger, "Operacion Desconocida: %d", cod_op);
        }
        }
    }
}