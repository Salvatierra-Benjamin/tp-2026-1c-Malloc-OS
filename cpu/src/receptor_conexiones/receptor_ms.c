#include "receptor_ms.h"
void receptor_ms()
{
    log_debug(cpu_logger, "[RECEPTOR_MS_CPU] Conectando con ms...");
    pthread_t hilo_atencion_ms;
    pthread_create(&hilo_atencion_ms, NULL, atencion_ms, NULL);
    pthread_join(hilo_atencion_ms, NULL); //! SE BLOQUEA ACA ESPECIFICAMENTE YA QUE DESPUES DE ESTE RECEPTOR NO HAY MAS CODIGO EN MAIN Y SE TERMINARIA TODO!!! PONER SOLO DETACH SI HAY UN ANCLA COMO UN WHILE EN EL MAIN!!!

    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_ms()
{
    log_debug(cpu_logger, "[RECEPTOR_MS_CPU] CPU CONECTADO CON MS");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_ms);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "MS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case MS_OK_CONEXION:
        {
            log_debug(cpu_logger, "[RECEPTOR_MS_CPU] EL HANDSHAKE CON MS SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_MS_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
