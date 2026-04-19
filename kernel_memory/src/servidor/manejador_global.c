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

    int cod_op = recibir_operacion(socket_cliente);

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
    case HANDSHAKE_IO: // BORRALO EN TU RAMA AL MERGEAR, LO ESCRIBI APROPOSITO PARA QUE BENJA HAGA LAS PRUEBAS DE IO HASTA QUE ESTE EL KS. NO TOQUE NADA MAS. SALUDOS :-)
    {

        log_info(logger, "Se conectó IO");
        enviar_operacion(socket_cliente, KM_OK_CONEXION);
        break;
    }
    case -1:
    {
        break;
    }
    default:
    {
        log_warning(logger, "se conecto un modulo desconocido : %d", socket_cliente);
        break;
    }
    }
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

    enviar_operacion(socket_cliente, KM_OK_CONEXION);

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
