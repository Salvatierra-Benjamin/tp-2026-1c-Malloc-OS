#include "receptor_ks.h"
void receptor_ks()
{
    log_debug(io_logger, "[RECEPTOR_KS_IO] Conectando con KS...");
    pthread_t hilo_atencion_ks;
    pthread_create(&hilo_atencion_ks, NULL, atencion_ks, NULL);
    pthread_join(hilo_atencion_ks, NULL);
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de ks y servidor de CPU al mismo tiempo).
}

void *atencion_ks()
{
    log_debug(io_logger, "[RECEPTOR_KS_IO] CONECTADO CON KS");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_ks);

        if (cod_op < 0)
        {
            log_error(io_logger, "cod op %d", cod_op);
            log_error(io_logger, "KS desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(io_logger, "[RECEPTOR_KS_IO] EL HANSHAKE CON KS SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        case -1:
        {
            log_error(io_logger, "Error de comunicación con la KS");
            io_liberar_config();
            io_liberar_log();
            exit(EXIT_FAILURE);
            break;
        }
        default:
        {
            log_warning(io_logger, "[RECEPTOR_KS_IO] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
