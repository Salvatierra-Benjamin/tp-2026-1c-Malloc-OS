#include "main_ms.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] [tamanio]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    tamanio_memoria_ms_ingresada = atoi(argv[2]);

    if (tamanio_memoria_ms_ingresada <= 0)
    {
        fprintf(stderr, "Error: El tamanio de memoria '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    ms_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_ms();
    inicializar_semaforos_coordinadores();
    memoria_reservada();
    creacion_conexiones_ms();

    log_debug(ms_logger, "Memory Stick TERMINO.\n");
    liberar_todo();

    return 0;
}
