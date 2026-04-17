#include "receptor_ms_km.h"
void receptor_ms_km(int conexion_km)
{
    int cod_op1 = recibir_operacion(conexion_km);
    if (cod_op1 < 0)
    {
        log_error(ms_logger, "cod op1 %d", cod_op1);
        log_error(ms_logger, "KM desconectado o error");
        exit(EXIT_FAILURE);
    }
    switch (cod_op1)
    {
    case MS_OK_CONEXION:
    {
        log_debug(ms_logger, "[RECEPTOR_KM] EL HANSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
        break;
    }
    default:
    {
        log_warning(ms_logger, "[RECEPTOR_KM] operacion desconocida : %d", cod_op1);
    }
    }
}