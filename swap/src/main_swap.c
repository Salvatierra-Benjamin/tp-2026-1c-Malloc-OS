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

    // El archivo tiene que estar abierto ANTES de conectarse a KM:
    // receptor_km() bloquea para siempre y las peticiones pueden llegar apenas
    // se completa el handshake.
    // "w+b" crea el archivo vacío si no existe, o lo trunca a 0 si existe.
    archivo_swap = fopen(swap_config.swap_file_path, "w+b");
    if (archivo_swap == NULL)
    {
        log_error(swap_logger, "No se pudo crear/abrir el archivo de SWAP");
        exit(EXIT_FAILURE);
    }

    // Truncamos/expandimos el archivo al tamaño total indicado en la config
    if (ftruncate(fileno(archivo_swap), swap_config.swap_file_size) != 0)
    {
        log_error(swap_logger, "No se pudo dimensionar el archivo de SWAP a %d bytes", swap_config.swap_file_size);
        exit(EXIT_FAILURE);
    }

    creacion_conexiones_swap();

    log_debug(swap_logger, "SWAP TERMINO.\n");
    liberar_todo();

    return 0;
}
