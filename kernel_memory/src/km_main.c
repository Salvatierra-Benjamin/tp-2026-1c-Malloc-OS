#include <km_main.h>

void *seleccion_dispositivo(void *socket_cliente)
{
    int socket = *((int *) socket_cliente);
    free(socket_cliente);
    while(1)
    {
        int cod_op = recibir_operacion(socket);
        if (cod_op <= 0)
        {
            close(socket);
            continue;
        }
        switch (cod_op)
        {
            case HANDSHAKE_KS:
            {
                log_info(logger, "se conecto KS");
            }

            case HANDSHAKE_IO:
            {
                log_info(logger, "se conecto IO");
            }

            case HANDSHAKE_CPU:
            {
                log_info(logger, "se conecto CPU");
            }

            case HANDSHAKE_SWAP:
            {
                log_info(logger, "se conecto SWAP");
            }
        }
    }
}

int main(int argc, char* argv[]) {

    t_config *config = config_create("/home/utnso/tp-2026-1c-Malloc-OS/kernel_memory/km.config");

    km_config *struct_config = iniciar_config(config);

    t_log_level log_level = log_level_from_string(struct_config->log_level);

    logger = log_create("km.log", "LOGGER - KM", 1, log_level);

    int socket_servidor = iniciar_servidor(struct_config->puerto_escucha);

    if (socket_servidor == -1)
    {
        log_info(logger, "No se pudo iniciar servidor");
    }

    while(1)
    {
        int socket_cliente = esperar_cliente(socket_servidor);
        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = socket_cliente;

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, seleccion_dispositivo, (void *)socket_ptr);
        pthread_detach(hilo_cliente);
    }

    return 0;
}
