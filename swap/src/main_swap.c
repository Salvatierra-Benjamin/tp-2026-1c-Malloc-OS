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

    int conexion_km = crear_conexion(swap_config.ip_km, swap_config.puerto_km);
    if (conexion_km == -1)
    {
        log_info(swap_logger, "ERROR AL CREAR CONEXION SWAP->KM.\n");
        swap_liberar_config();
        swap_liberar_log();
        return -1;
    }
    enviar_operacion(conexion_km, HANDSHAKE_SWAP); // Nuestro Handshake con KM

    receptor_km(conexion_km);

    log_debug(swap_logger, "SWAP TERMINO.\n");

    swap_liberar_config();
    swap_liberar_log();

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
