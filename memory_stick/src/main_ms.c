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
    int tamanio_memoria = atoi(argv[2]);

    if (tamanio_memoria <= 0)
    {
        fprintf(stderr, "Error: El tamanio de memoria '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    ms_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config();

    void *memoria = malloc(tamanio_memoria);
    if (memoria == NULL)
    {
        log_debug(ms_logger, "Error al hacer malloc de la memoria principal.\n");
        return EXIT_FAILURE;
    }
    log_debug(ms_logger, "Memoria de %d bytes reservada con éxito.\n", tamanio_memoria);

    int conexion_ms = crear_conexion(ms_config.ip_km, ms_config.puerto_km);
    if (conexion_ms == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION MS->KM.\n");
        ms_liberar_config();
        ms_liberar_log();
        return -1;
    }
    enviar_operacion(conexion_ms, HANDSHAKE_MS); // Nuestro Handshake con KM

    log_debug(ms_logger, "Memory Stick TERMINO.\n");

    ms_liberar_config();
    ms_liberar_log();
    free(memoria);

    return 0;
}
