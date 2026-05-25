#include "receptor_debug_server.h"
void receptor_debug_server()
{
    log_debug(debug_client_logger, "[RECEPTOR_debug_server_debug_client] Conectando con debug_server...");
    pthread_t hilo_atencion_debug_server;
    pthread_create(&hilo_atencion_debug_server, NULL, atencion_debug_server, NULL);
    pthread_detach(hilo_atencion_debug_server);
}

void *atencion_debug_server()
{
    log_debug(debug_client_logger, "[RECEPTOR_debug_server_debug_client] debug_client CONECTADO CON debug_server");

    while (1)
    {

        int cod_op = recibir_operacion(conexion_debug_server);

        if (cod_op < 0)
        {
            log_error(debug_client_logger, "cod op %d", cod_op);
            log_error(debug_client_logger, "debug_server desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case ALOJAR_PROCESO:
        {

            t_paquete *paquete_alojar_proceso = recibir_paquete(conexion_debug_server);
            buffer_read(&PID_a_ejecutar, paquete_alojar_proceso->buffer, sizeof(int));
            eliminar_paquete(paquete_alojar_proceso);
            sem_post(&sem_proceso_nuevo);
            break;
        }
        // case INTERRUPCION:
        // {
        //     - El Kernel te pide que desalojes.
        //     - Solo tenés que prender una variable global (un flag atómico)
        //     - que tu main va a leer en la etapa de "Check Interrupt"
        //     hubo_interrupcion = true;
        //     break;
        // }
        default:
        {
            log_warning(debug_client_logger, "[RECEPTOR_debug_server_debug_client] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
