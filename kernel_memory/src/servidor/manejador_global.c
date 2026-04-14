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
            log_info(logger, "se conecto ks");

            int *socket_ks_ptr = malloc(sizeof(int));
            if (socket_ks_ptr == NULL)
            {
                log_error(logger, "Error al alocar memoria para socket");
                close(socket_cliente);
                continue;
            }
            *socket_ks_ptr = socket_cliente;

            pthread_t hilo_ks;
            if (pthread_create(&hilo_ks, NULL, manejador_ks, socket_ks_ptr) != 0)
            {
                log_error(logger, "Error al crear hilo para kernel memory");
                free(socket_ks_ptr);
                close(socket_cliente);
                continue;
            }
            pthread_detach(hilo_ks);
            break;
        }
        case HANDSHAKE_CPU:
        {
            log_info(logger, "se conecto cpu");
            break;
        }
        case HANDSHAKE_IO:
        {
            log_info(logger, "se conecto io");
            break;
        }
        case HANDSHAKE_SWAP:
        {
            log_info(logger, "se conecto swap");
            break;
        }
        }
    }

    close(socket_cliente);
    return NULL;
}