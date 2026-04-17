#include "receptor_ms.h"
void *receptor_ms(void *arg)
{
    while (1)
    {
        // pthread_mutex_lock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!
        int cod_op1 = recibir_operacion(parametros_ms.conexion_ms);
        // pthread_mutex_unlock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!
        if (cod_op1 < 0)
        {
            log_error(cpu_logger, "cod op1 %d", cod_op1);
            log_error(cpu_logger, "MS desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op1)
        {
        case CPU_OK_CONEXION:
        {
            log_debug(cpu_logger, "[RECEPTOR_MS] EL HANSHAKE CON MS SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_MS] operacion desconocida : %d", cod_op1);
        }
        }
    }
}