#include "receptor_ks.h"
void receptor_ks()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] Conectando con KS...");
    pthread_t hilo_atencion_ks;
    pthread_create(&hilo_atencion_ks, NULL, atencion_ks, NULL);
    pthread_detach(hilo_atencion_ks);
}

void *atencion_ks()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] CPU CONECTADO CON KS");

    while (1)
    {

        int cod_op = recibir_operacion(conexion_ks);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case ALOJAR_PROCESO:
        {
            t_paquete *paquete_alojar_proceso = recibir_paquete(conexion_ks);
            buffer_read(&PID_a_ejecutar, paquete_alojar_proceso->buffer, sizeof(int));
            eliminar_paquete(paquete_alojar_proceso);
            sem_post(&sem_proceso_nuevo);
            break;
        }
        case INTERRUPCION:
        {
            t_paquete *paquete_interrupcion = recibir_paquete(conexion_ks);
            buffer_read(&PID_Interrupcion, paquete_interrupcion->buffer, sizeof(int));
            eliminar_paquete(paquete_interrupcion);
            sem_post(&hay_interrupcion);
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KS_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
