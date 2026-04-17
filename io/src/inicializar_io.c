#include "inicializar_io.h"
t_log *io_logger;
io_config_struct io_config;
char *normalizar(char *tipo_io)
{
    char *aux = string_duplicate(tipo_io);

    string_to_upper(aux);
    return aux;
}

void imprimir_tipo_de_io(char *argumento_mayusculas)
{
    printf("IO del tipo: %s \n", argumento_mayusculas);
}

void io_cargar_config(char *path)
{

    t_config *config = config_create(path);
    if (!config)
    {
        fprintf(stderr, "ERROR AL CREAR CONFIG DE IO\n");
        exit(EXIT_FAILURE);
    }

    char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    io_config.log_level = log_level_from_string((char *)log_level_string);
    io_config.ip_ks = strdup(config_get_string_value(config, "IP_KS"));
    io_config.puerto_ks = strdup(config_get_string_value(config, "PUERTO_KS"));

    config_destroy(config);
}

void inicializar_logger()
{
    io_logger = log_create("io.log", "IO", 1, io_config.log_level);

    if (io_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER IO\n");
        exit(EXIT_FAILURE);
    }
    log_debug(io_logger, "Se creo log de IO correctamente");
}

void imprimir_valores_config_io()
{
    // log_debug(swap_logger, "Config y Logger del SWAP creados correctamente.\n");
    log_debug(io_logger, "Valores cargados en Config de IO:");
    log_debug(io_logger, "Nivel de Log: %d", io_config.log_level);
    log_debug(io_logger, "IP KS: %s", io_config.ip_ks);
    log_debug(io_logger, "Puerto KS: %s", io_config.puerto_ks);
}

void io_liberar_config()
{
    free(io_config.ip_ks);
    free(io_config.puerto_ks);
    log_debug(io_logger, "Estructura de la Config Liberada\n");
}

void io_liberar_log()
{
    log_debug(io_logger, "Log Liberado!");
    log_destroy(io_logger);
}