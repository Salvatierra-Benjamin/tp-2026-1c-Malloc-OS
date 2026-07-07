#include "receptor_km.h"
// TODO Implementar delay
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

void *atencion_km(void *arg)
{
    log_debug(ms_logger, "[RECEPTOR_KM_MS] CONECTADO CON KM");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(ms_logger, "cod op %d", cod_op);
            log_error(ms_logger, "KM desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_OK:
        {
            log_debug(ms_logger, "[RECEPTOR_KM_MS] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");

            // Conexión a Kernel Memory: “## Conectado a Kernel Memory”
            log_info(ms_logger, "## Conectado a Kernel Memory");

            sem_post(&sem_ok_conexion_km);

            t_paquete *paquete_tamaño_puerto_km = crear_paquete(MEMORY_STICK_SIZE);

            agregar_a_paquete(paquete_tamaño_puerto_km, &tamanio_memoria_ms_ingresada, sizeof(int));

            int tamanio_mi_ip = strlen(ms_config.mi_ip) + 1;
            agregar_a_paquete(paquete_tamaño_puerto_km, &tamanio_mi_ip, sizeof(int));
            agregar_a_paquete(paquete_tamaño_puerto_km, ms_config.mi_ip, tamanio_mi_ip);

            int tamanio_puerto = strlen(ms_config.puerto_escucha_cpu) + 1;
            agregar_a_paquete(paquete_tamaño_puerto_km, &tamanio_puerto, sizeof(int));
            agregar_a_paquete(paquete_tamaño_puerto_km, ms_config.puerto_escucha_cpu, tamanio_puerto);

            enviar_paquete(paquete_tamaño_puerto_km, conexion_km);
            eliminar_paquete(paquete_tamaño_puerto_km);

            break;
        }
        // NOTA: las R/W de KM NO llegan por esta conexión (la que MS inició hacia KM).
        // KM abre una conexión de datos dedicada al puerto de escucha de CPUs del stick
        // (sin handshake) y manda PETICION_ESCRITURA_MS / PETICION_LECTURA_MS por ahí,
        // así que las atiende esperar_cpus.c/atencion_cpus (que loguea Escritura/Lectura).
        // Esta conexión solo transporta el handshake + MEMORY_STICK_SIZE.
        default:
        {
            log_warning(ms_logger, "[RECEPTOR_KM_MS] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
