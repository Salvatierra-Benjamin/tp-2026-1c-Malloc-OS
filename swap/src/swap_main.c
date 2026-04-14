#include <swap_utils.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ARGUMENTOS INSUFICIENTES\n");
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    swap_config_struct swap_config;
    t_log *swap_logger;
    swap_cargar_config(path, &swap_config);

    swap_logger = log_create("swap.log", "SWAP", 1, swap_config.log_level);
    if (swap_logger == NULL)
    {
        fprintf(stderr, "ERROR AL CREAR EL LOGGER SWAP\n");
        swap_liberar_config(&swap_config,swap_logger);
        return -1;
    }
  
    log_debug(swap_logger,"Config y Logger del SWAP creados correctamente.\n");
    log_debug(swap_logger, "Valores cargados en Swap:");
    log_debug(swap_logger, "Path Archivo: %s", swap_config.swap_file_path);
    log_debug(swap_logger, "Tamaño Archivo: %d", swap_config.swap_file_size);
    log_debug(swap_logger, "Tamaño Bloque: %d", swap_config.swap_block_size);
    log_debug(swap_logger, "IP Kernel: %s", swap_config.ip_km);
    log_debug(swap_logger, "Puerto Kernel: %s", swap_config.puerto_km);
    log_debug(swap_logger, "Nivel de Log: %d\n", swap_config.log_level);

    int conexion_swap = crear_conexion(swap_config.ip_km, swap_config.puerto_km);
    if (conexion_swap == -1)
    {
        log_info(swap_logger, "ERROR AL CREAR CONEXION SWAP->KM.\n");
        swap_liberar_config(&swap_config,swap_logger);
        swap_liberar_log(swap_logger);
        return -1;
    }
    enviar_operacion(conexion_swap, HANDSHAKE_SWAP); // Nuestro Handshake con KM

    log_debug(swap_logger, "SWAP TERMINO.\n");
    
    swap_liberar_config(&swap_config,swap_logger);
    swap_liberar_log(swap_logger);
    return 0;
}
//! SEBA NO LO BORRES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! >:/
 // parametros_swap parametros_hilo;
    // parametros_hilo.swap_file_size = swap_config.swap_file_size
    // parametros_hilo.swap_block_size = swap_config.swap_block_size
    // pthread_t hilo_km;
    // if (pthread_create(&hilo_km, NULL, receptor_km, (void *)&parametros_hilo) != 0)
    // {
    //     // t_paquete paquete_tamanio = creat
    //     perror("Error al crear hilo storage");
    //     exit(EXIT_FAILURE);
    // }
    // pthread_join(hilo_km, NULL);
