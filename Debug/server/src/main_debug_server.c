#include "main_debug_server.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] \n", argv[0]);
        return EXIT_FAILURE;
    }
    char *path = argv[1];

    debug_server_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_debug_server();
    inicializar_semaforos_coordinadores();
    creacion_conexiones_debug_server();

    log_debug(debug_server_logger, "debug_server TERMINO.\n");
    liberar_todo();

    return 0;
}
