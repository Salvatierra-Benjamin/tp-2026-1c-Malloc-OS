#include <sp_main.h>

int main(int argc, char *argv[])
{

    char *path = argv[1];
    swap_config swap_config;
    t_log *swap_logger;
    swap_cargar_config(path, &swap_config);

    swap_logger = log_create("swap.log", "SWAP", 1, swap_config.log_level);
    if (swap_logger == NULL)
    {
        printf("ERROR AL CREAR EL LOGGER SWAP\n");
        swap_liberar_config(&swap_config);
        return -1;
    }

    int conexion_sp = crear_conexion(swap_config.ip_km, swap_config.puerto_km);
    if (conexion_sp == -1)
    {
        log_info(swap_logger, "ERROR AL CREAR CONEXION SWAP->KM\n");
        swap_liberar_config(&swap_config);
        swap_liberar_log(swap_logger);
        return -1;
    }
    enviar_operacion(conexion_sp, HANDSHAKE_SWAP); // Nuestro Handshake con KM

    log_debug(swap_logger, "SWAP TERMINO");

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

    swap_liberar_config(&swap_config);
    swap_liberar_log(swap_logger);

    return 0;
}
