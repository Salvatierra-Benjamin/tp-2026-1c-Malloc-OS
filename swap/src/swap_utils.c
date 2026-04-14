#include <swap_utils.h>
void swap_cargar_config(char *path, swap_config_struct *swap_config)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE SWAP\n");
        exit(EXIT_FAILURE);
    }

    swap_config->swap_file_path = strdup(config_get_string_value(config, "SWAP_FILE_PATH"));
    swap_config->swap_file_size = config_get_int_value(config, "SWAP_FILE_SIZE");
    swap_config->swap_block_size = config_get_int_value(config, "BLOCK_SIZE");
    swap_config->ip_km = strdup(config_get_string_value(config, "IP_KM"));
    swap_config->puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    swap_config->log_level = log_level_from_string((char *)log_level_string);

    config_destroy(config);

}

void swap_liberar_config(swap_config_struct *swap_config,t_log *swap_logger)
{
    free(swap_config->swap_file_path);
    free(swap_config->ip_km);
    free(swap_config->puerto_km);
   
    log_debug(swap_logger,"Estructura de la Config Liberada\n");
}

void swap_liberar_log(t_log *swap_logger)
{
    log_debug(swap_logger,"Log Liberado!!!\n");
    log_destroy(swap_logger);
    
}