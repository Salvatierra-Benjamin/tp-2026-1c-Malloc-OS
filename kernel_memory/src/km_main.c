#include <km_main.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "argumentos insuficientes\n");
        fprintf(stderr, "./bin/kernel_memory <config_name>\n");
        return EXIT_FAILURE;
    }
    char *config_name = argv[1];

    config = config_create(config_name);
    km_config *parametros = iniciar_config(config);
    logger = log_create("km.log", "LOGGER - KM", 1, parametros->log_level);

    log_info(logger, "Inicio Kernel Memory");

    int socket_servidor = iniciar_servidor(parametros->puerto_escucha);

    if (socket_servidor == -1)
    {
        log_error(logger, "No se pudo iniciar servidor");
        exit(0);
    }

    entrada_dispositivos(socket_servidor);

    liberar_recursos();

    return 0;
}
