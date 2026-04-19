#include "receptor_km.h"
void receptor_km()
{
    log_debug(swap_logger, "[RECEPTOR_KM_SWAP] Conectando con KM...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_join(hilo_atencion_km, NULL);
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_km()
{
    log_debug(swap_logger, "[RECEPTOR_KM_SWAP] CONECTADO CON KM");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(swap_logger, "cod op %d", cod_op);
            log_error(swap_logger, "KM desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(swap_logger, "[RECEPTOR_KM_SWAP] EL HANSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        case -1:
        {
            log_error(swap_logger, "Error de comunicación con la KM");
            swap_liberar_config();
            swap_liberar_log();
            exit(EXIT_FAILURE);
            break;
        }
        default:
        {
            log_warning(swap_logger, "[RECEPTOR_KM_SWAP] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
