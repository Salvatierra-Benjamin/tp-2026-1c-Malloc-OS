#include "receptor_ks.h"
void receptor_ks()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] Conectando con KS...");
    pthread_t hilo_atencion_ks;
    pthread_create(&hilo_atencion_ks, NULL, atencion_ks, NULL);
    pthread_detach(hilo_atencion_ks);

    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_ks()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] CPU CONECTADO CON KS");

    while (1)
    {

        int cod_op = recibir_operacion(conexion_ks);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KS_OK_CONEXION:
        {
            log_debug(cpu_logger, "[RECEPTOR_KS_CPU] EL HANDSHAKE CON KS SE HA REALIZADO CORRECTAMENTE!!!");
            break;
        }
        case ALOJAR_PROCESO:
        {

            t_paquete *paquete_alojar_proceso = recibir_paquete(conexion_ks);

            int pid;
            buffer_read(&pid, paquete_alojar_proceso->buffer, sizeof(int));

            t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
            agregar_a_paquete(paquete_solicitud_contexto, &pid, sizeof(int));

            enviar_paquete(paquete_solicitud_contexto, conexion_km);

            eliminar_paquete(paquete_alojar_proceso);
            eliminar_paquete(paquete_solicitud_contexto);
            // ? no poner semaforo!
            // ? el ks solicita alojar un proceso, hasta que cpu envia ok no deberia de mandar nada mas ks
            // ? No poner semaforo para receptor ks porque solo taparia el error en caso de que ks
            // ? siga enviando cosas
            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KS_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
