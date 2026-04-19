#include "receptor_km.h"
void receptor_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KM_CPU] Conectando con KM...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_detach(hilo_atencion_km);

    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KM_CPU] CPU CONECTADO CON KM");

    while (1)
    {
        // pthread_mutex_lock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!
        int cod_op = recibir_operacion(conexion_km);
        // pthread_mutex_unlock(&mutex_conexion); <-- NO TOCAR ES PARA DESPUÉS!!!

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KM desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(cpu_logger, "[RECEPTOR_KM_CPU] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        case -1:
        {
            log_error(cpu_logger, "Error de comunicación con la KM");
            cpu_liberar_config();
            cpu_liberar_log();
            exit(EXIT_FAILURE);
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KM_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
