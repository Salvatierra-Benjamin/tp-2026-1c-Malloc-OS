#include <ms_utils.h>

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
    
    if (tamanio_memoria <= 0) {
        fprintf(stderr, "Error: El tamanio de memoria '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    ms_config_struct ms_config;
    t_log *ms_logger;
    ms_cargar_config(path, &ms_config);

    ms_logger = log_create("ms.log", "ms", 1, ms_config.log_level);
    if (ms_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER ms\n");
        ms_liberar_config(&ms_config,ms_logger);
        return -1;
    }
  
    log_debug(ms_logger,"Config y Logger del ms creados correctamente.\n");
    log_debug(ms_logger, "Valores cargados en Memoria:");
    log_debug(ms_logger, "Delay Memoria: %d", ms_config.memoria_delay);
    log_debug(ms_logger, "IP Kernel: %s", ms_config.ip_km);
    log_debug(ms_logger, "Puerto Kernel: %s", ms_config.puerto_km);
    log_debug(ms_logger, "Nivel de Log: %d\n", ms_config.log_level);

    void* memoria = malloc(tamanio_memoria);
    if (memoria == NULL) {
        perror("Error al hacer malloc de la memoria principal.\n");
        return EXIT_FAILURE;
    }
    log_debug(ms_logger,"Memoria de %d bytes reservada con éxito.\n", tamanio_memoria);

    int conexion_ms = crear_conexion(ms_config.ip_km, ms_config.puerto_km);
    if (conexion_ms == -1)
    {
        log_info(ms_logger, "ERROR AL CREAR CONEXION MS->KM.\n");
        ms_liberar_config(&ms_config,ms_logger);
        ms_liberar_log(ms_logger);
        return -1;
    }
    enviar_operacion(conexion_ms, HANDSHAKE_MS); // Nuestro Handshake con KM

    log_debug(ms_logger, "Memory Stick TERMINO.\n");

    ms_liberar_config(&ms_config,ms_logger);
    ms_liberar_log(ms_logger);
    free(memoria);

    return 0;
}
