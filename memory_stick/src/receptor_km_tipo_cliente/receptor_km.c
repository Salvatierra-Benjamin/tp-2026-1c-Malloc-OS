#include "receptor_km.h"
void receptor_km()
{
    log_debug(ms_logger, "[RECEPTOR_KM_MS] Conectando con KM...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_detach(hilo_atencion_km); // Cuando este hilo termine, que nadie lo espere; libera su memoria automáticamente: pthread_detach(hilo_atencion_km);
    // Si fuera pthread_join (hilo_atencion_km,NULL); dejaria de ser multihilo ya que esta funcion es bloqueante entonces la CPU 2 debe esperar a que termine la CPU 1.
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_km()
{
    log_debug(ms_logger, "[RECEPTOR_KM_MS] CONECTADO CON KM");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(ms_logger, "cod op %d", cod_op);
            log_error(ms_logger, "KM desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(ms_logger, "[RECEPTOR_KM_MS] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        case -1:
        {
            log_error(ms_logger, "Error de comunicación con la KM");
            ms_liberar_config();
            ms_liberar_log();
            free(memoria_ms);
            exit(EXIT_FAILURE);
            break;
        }
        default:
        {
            log_warning(ms_logger, "[RECEPTOR_KM_MS] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
