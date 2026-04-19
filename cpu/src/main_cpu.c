#include "main_cpu.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] [Identificador]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    int identificador = atoi(argv[2]);

    if (identificador <= 0)
    {
        fprintf(stderr, "Error: El identificador '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    cpu_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_cpu();
    creacion_conexiones_cpu();

    log_debug(cpu_logger, "Memory Stick TERMINO.\n");

    cpu_liberar_config();
    cpu_liberar_log();

    return 0;
}
