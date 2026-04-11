 #include <sp_utils.h>
swap_config swap_cargar_config_log(char* path){

    swap_config* cfg = config_create(path);
    if(!cfg) return false;
    
    cfg.swap_file_path = strdup(config_get_string_value(cfg,"SWAP_FILE_PATH")); // FIFO | PRIORIDADES
    cfg.swap_file_size = config_get_int_value(cfg,"SWAP_FILE_SIZE");
    cfg.swap_block_size= config_get_int_value(cfg,"BLOCK_SIZE");
    cfg.puerto_km = strdup(config_get_string_value(cfg,"PUERTO"));
    cfg.ip_km= config_get_int_value(cfg,"IP");

    const char* log_level_string = config_get_string_value(cfg, "LOG_LEVEL");
    t_log_level level = log_level_from_string((char*)log_level_string);
    t_log sp_logger = log_create("swap.log","SWAP",1, level);

    return cfg;
}