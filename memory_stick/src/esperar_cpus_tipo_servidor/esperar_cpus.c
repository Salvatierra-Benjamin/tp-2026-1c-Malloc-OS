#include "esperar_cpus.h"
void esperar_cpus()
{
    while (1)
    {
        log_debug(ms_logger, "Esperando cpus...");
        int socket_cliente = esperar_cliente(socket_servidor_con_cpu);
        int *socket_cliente_puntero = malloc(sizeof(int));
        *socket_cliente_puntero = socket_cliente;
        pthread_t hilo_atencion_cpus;
        pthread_create(&hilo_atencion_cpus, NULL, atencion_cpus, (void *)socket_cliente_puntero);
        pthread_detach(hilo_atencion_cpus); // Cuando este hilo termine, que nadie lo espere; libera su memoria automáticamente.
        //  Si fuera pthread_join (hilo_atencion_km,NULL); dejaria de ser multihilo ya que esta funcion es bloqueante entonces la CPU 2 debe esperar a que termine la CPU 1.
        // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
        // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
    }
}

void *atencion_cpus(void *socket_cliente_puntero)
{
    log_debug(ms_logger, "Se conecto un CPU");
    int socket_cliente = *((int *)(socket_cliente_puntero));
    free(socket_cliente_puntero);

    while (1)
    {

        int cod_op = recibir_operacion(socket_cliente);

        if (cod_op < 0)
        {
            log_error(ms_logger, "cod op %d", cod_op);
            log_error(ms_logger, "CPU desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_CPU:
        {
            t_paquete *paquete_handshake_ms = recibir_paquete(socket_cliente);
            int cpu_id;
            buffer_read(&cpu_id, paquete_handshake_ms->buffer, sizeof(int));
            eliminar_paquete(paquete_handshake_ms);
            log_debug(ms_logger, "[RECEPTOR_CPUS]Recibido el cpu_id número: %d", cpu_id);
            enviar_operacion(socket_cliente, MS_OK_CONEXION);

            break;
        }
        default:
        {
            log_warning(ms_logger, "[RECEPTOR_CPUS]Operacion Desconocida: %d", cod_op);
            break;
        }
        }
    }
}
