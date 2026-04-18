#include <km_utils.h>

km_config *iniciar_config(t_config *config)
{
    km_config *struct_config = malloc(sizeof(km_config));
    struct_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    char *log_level_string = strdup(config_get_string_value(config, "LOG_LEVEL"));
    struct_config->log_level = log_level_from_string(log_level_string);
    struct_config->segment_max_size = config_get_int_value(config, "SEGMENT_MAX_SIZE");
    struct_config->allocation_strategy = strdup(config_get_string_value(config, "ALLOCATION_STRATEGY"));
    struct_config->instruction_delay = config_get_int_value(config, "INSTRUCTION_DELAY");
    struct_config->compaction_delay = config_get_int_value(config, "COMPACTION_DELAY");
    struct_config->scripts_basepath = strdup(config_get_string_value(config, "SCRIPTS_BASEPATH"));
    free(log_level_string);
    return struct_config;
}
