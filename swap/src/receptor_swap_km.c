#include "receptor_swap_km.h"
void receptor_swap_km(int conexion_km)
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
    case SWAP_OK_CONEXION:
    {
        log_debug(swap_logger, "[RECEPTOR_KM] EL HANSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
        break;
    }
    default:
    {
        log_warning(swap_logger, "[RECEPTOR_KM] operacion desconocida : %d", cod_op1);
    }
    }
}
