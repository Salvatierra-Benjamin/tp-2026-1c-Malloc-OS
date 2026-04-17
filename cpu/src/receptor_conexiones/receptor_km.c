#include "receptor_km.h"
void *receptor_km(void *arg)
{
    while (1)
    {
        // pthread_mutex_lock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!
        int cod_op1 = recibir_operacion(parametros_km.conexion_km);
        // pthread_mutex_unlock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!
        if (cod_op1 < 0)
        {
            log_error(cpu_logger, "cod op1 %d", cod_op1);
            log_error(cpu_logger, "KM desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op1)
        {
        case KM_OK_CONEXION:
        {
            t_paquete *ok_conexion = recibir_paquete(parametros_km.conexion_km);
            eliminar_paquete(ok_conexion);
            log_debug(cpu_logger, "[RECEPTOR_KM] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KM] operacion desconocida : %d", cod_op1);
        }
        }
    }
}
