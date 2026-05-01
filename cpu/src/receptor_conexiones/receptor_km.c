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

        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KM desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(cpu_logger, "[RECEPTOR_KM_CPU] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            // enviar paquete con operacion de solicitud contexto ejecucicion , con el pid que guardaste en ks

            break;
        }
        case CONTEXTO_EJECUCION:
        {

            t_paquete *paquete_contexto_de_ejecucion = recibir_paquete(conexion_km);

            t_contexto_ejecucion nuevo_contexto_ejecucion;
            buffer_read(&nuevo_contexto_ejecucion, paquete_contexto_de_ejecucion->buffer, sizeof(nuevo_contexto_ejecucion));

            // No hay necesidad que sea puntero contexto_ejecucion que sea puntero, siempre sera
            // del mismo tamaño. t_list* solo tendra un nodo tras otro, problema del nodo, no se guarda en la estructura
            contexto_ejecucion.PID = nuevo_contexto_ejecucion.PID;
            contexto_ejecucion.registros = nuevo_contexto_ejecucion.registros;
            contexto_ejecucion.tabla_segmentos = nuevo_contexto_ejecucion.tabla_segmentos;

            // !Poner semaforo para resguardar la conexion con ks
            t_paquete *paquete_ok_alojo = crear_paquete(OK_ALOJAR);
            agregar_a_paquete(paquete_ok_alojo, &contexto_ejecucion.PID, sizeof(int));

            enviar_paquete(paquete_ok_alojo, conexion_ks);

            eliminar_paquete(paquete_ok_alojo);
            eliminar_paquete(paquete_contexto_de_ejecucion);

            iniciar_fetch();
            // ? el ks solicita alojar un proceso, hasta que el
            // ? No poner semaforo para receptor ks porque solo taparia el error en caso de que ks
            // ? siga enviando cosas
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KM_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
