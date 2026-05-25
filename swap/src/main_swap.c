#include "main_swap.h"

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "ARGUMENTOS INSUFICIENTES\n");
        return EXIT_FAILURE;
    }
    char *path = argv[1];

    swap_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_swap();
    inicializar_semaforos_coordinadores();
    creacion_conexiones_swap();

    log_debug(swap_logger, "SWAP TERMINO.\n");
    liberar_todo();

    return 0;
}
