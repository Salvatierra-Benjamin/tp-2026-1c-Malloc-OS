#include <ms_utils.h>
void ms_cargar_config(char *path, ms_config_struct *ms_config)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr,"ERROR AL CREAR CONFIG DE MEMORY STICK\n");
        exit(EXIT_FAILURE);
    }

    ms_config->memoria_delay = config_get_int_value(config, "MEMORY_DELAY");
    ms_config->ip_km = strdup(config_get_string_value(config, "IP_KM"));
    ms_config->puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    ms_config->log_level = log_level_from_string((char *)log_level_string);

    config_destroy(config);

}

void ms_liberar_config(ms_config_struct *ms_config,t_log *ms_logger)
{
    free(ms_config->ip_km);
    free(ms_config->puerto_km);
   
    log_debug(ms_logger,"Estructura de la Config Liberada\n");
}

void ms_liberar_log(t_log *ms_logger)
{
    log_debug(ms_logger,"Log Liberado!!!\n");
    log_destroy(ms_logger);
}