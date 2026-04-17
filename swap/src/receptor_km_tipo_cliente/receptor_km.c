#include "receptor_km.h"
void receptor_km(int conexion_km)
{
    int cod_op1 = recibir_operacion(conexion_km);
    if (cod_op1 < 0)
    {
        log_error(swap_logger, "cod op1 %d", cod_op1);
        log_error(swap_logger, "KM desconectado o error");
        exit(EXIT_FAILURE);
    }
    switch (cod_op1)
    {
    case KM_OK_CONEXION:
    {
        t_paquete *ok_conexion = recibir_paquete(conexion_km);
        eliminar_paquete(ok_conexion);
        log_debug(swap_logger, "[RECEPTOR_KM] EL HANSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
        break;
    }
    default:
    {
        log_warning(swap_logger, "[RECEPTOR_KM] operacion desconocida : %d", cod_op1);
    }
    }
}
