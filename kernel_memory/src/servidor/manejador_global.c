#include "manejador_global.h"

void entrada_dispositivos(int socket_servidor)
{
    while (1)
    {
        int socket_cliente = esperar_cliente(socket_servidor);
        if (socket_cliente == -1)
        {
            log_error(logger, "Error al aceptar cliente");
            continue;
        }

        int *socket_ptr = malloc(sizeof(int));
        if (socket_ptr == NULL)
        {
            log_error(logger, "Error al alocar memoria para socket");
            close(socket_cliente);
            continue;
        }
        *socket_ptr = socket_cliente;

        pthread_t hilo_cliente;
        if (pthread_create(&hilo_cliente, NULL, seleccion_dispositivo, socket_ptr) != 0)
        {
            log_error(logger, "Error al crear hilo para cliente");
            free(socket_ptr);
            close(socket_cliente);
            continue;
        }
        pthread_detach(hilo_cliente);
    }
}

void *seleccion_dispositivo(void *arg)
{
    int socket_cliente = *(int *)arg;
    free(arg);

    while (1)
    {
        int cod_op = recibir_operacion(socket_cliente);
        if (cod_op == -1)
        {
            log_warning(logger, "dispositivo se desconecto");
            break;
        }

        switch (cod_op)
        {
        case HANDSHAKE_KS:
        {
            delegar_a(socket_cliente, manejador_ks, "Kernel Scheduler");
            break;
        }
        case HANDSHAKE_CPU:
        {
            delegar_a(socket_cliente, manejador_cpu, "CPU");
            break;
        }
        case HANDSHAKE_SWAP:
        {
            delegar_a(socket_cliente, manejador_swap, "SWAP");
            break;
        }
        case HANDSHAKE_MS:
        {
            delegar_a(socket_cliente, manejador_memory_stick, "Memory Stick");
            break;
        }
        default:
        {
            log_warning(logger, "se conecto un modulo desconocido : %d", socket_cliente);
            break;

            //! pthread_detach(hilo);
            //!  TE LO COMENTE PORQUE ROMPIA TODO, ADEMÁS FLATABA UNA LLAVE.
        }
        }
    }
    close(socket_cliente); //! ESTO LO MOVI ACÁ COMO ESTA EN TU VERSION, TAL VEZ EL MERGEO LO MEZCLO.
    return NULL;
}
void delegar_a(int socket_cliente, void *(*manejador)(void *), const char *nombre_modulo)
{
    log_info(logger, "Se conectó %s", nombre_modulo);

    int *socket_ptr = malloc(sizeof(int));
    if (socket_ptr == NULL)
    {
        log_error(logger, "Error al alocar memoria para socket de %s", nombre_modulo);
        close(socket_cliente);
        return;
    }
    *socket_ptr = socket_cliente;

    t_paquete *ok_conexion = crear_paquete(KM_OK_CONEXION);
    enviar_paquete(ok_conexion, socket_cliente);
    eliminar_paquete(ok_conexion);

    pthread_t hilo;
    if (pthread_create(&hilo, NULL, manejador, socket_ptr) != 0)
    {
        log_error(logger, "Error al crear hilo para %s", nombre_modulo);
        free(socket_ptr);
        close(socket_cliente);
        return;
    }
    pthread_detach(hilo);
}
