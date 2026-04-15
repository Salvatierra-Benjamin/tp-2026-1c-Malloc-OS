#include "iniciador_swap.h"
t_log *swap_logger;
swap_config_struct swap_config;

void swap_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE SWAP\n");
        exit(EXIT_FAILURE);
    }

    swap_config.swap_file_path = strdup(config_get_string_value(config, "SWAP_FILE_PATH"));
    swap_config.swap_file_size = config_get_int_value(config, "SWAP_FILE_SIZE");
    swap_config.swap_block_size = config_get_int_value(config, "BLOCK_SIZE");
    swap_config.ip_km = strdup(config_get_string_value(config, "IP_KM"));
    swap_config.puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    swap_config.log_level = log_level_from_string((char *)log_level_string);

    config_destroy(config);
}
void inicializar_logger()
{
    swap_logger = log_create("swap.log", "SWAP", 1, swap_config.log_level);

    if (swap_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER SWAP\n");
        exit(EXIT_FAILURE);
    }
    log_debug(swap_logger, "Se creo log de Swap correctamente");
}
void imprimir_valores_config()
{
    // log_debug(swap_logger, "Config y Logger del SWAP creados correctamente.\n");
    log_debug(swap_logger, "Valores cargados en Swap:");
    log_debug(swap_logger, "Path Archivo: %s", swap_config.swap_file_path);
    log_debug(swap_logger, "Tamaño Archivo: %d", swap_config.swap_file_size);
    log_debug(swap_logger, "Tamaño Bloque: %d", swap_config.swap_block_size);
    log_debug(swap_logger, "IP Kernel: %s", swap_config.ip_km);
    log_debug(swap_logger, "Puerto Kernel: %s", swap_config.puerto_km);
    log_debug(swap_logger, "Nivel de Log: %d\n", swap_config.log_level);
}

void swap_liberar_config()
{
    free(swap_config.swap_file_path);
    free(swap_config.ip_km);
    free(swap_config.puerto_km);

    fprintf(stderr, "Estructura de la Config Liberada\n");
}

void swap_liberar_log()
{
    log_debug(swap_logger, "Log Liberado!!!\n");
    log_destroy(swap_logger);
}