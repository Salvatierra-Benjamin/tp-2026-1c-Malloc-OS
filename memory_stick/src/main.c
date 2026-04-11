#include <main.h>

int main(int argc, char* argv[]) {
   
    char* path = arg[1];
    t_config cfg = swap_cargar_config(path)

    km_config *struct_config = iniciar_config(config);

    t_log_level log_level = log_level_from_string(struct_config->log_level);

    logger = log_create("sp.log", "LOGGER - SWAPP", 1, log_level);


int conexion_sp = crear_conexion(cfg.ip_km , cfg.puerto_km);
enviar_operacion(conexion_sp, HANDSHAKE_SWAP);   // Nuestro Handshake Storage





// parametros_swap parametros_hilo;
// parametros_hilo.swap_file_size = cfg.swap_file_size
// parametros_hilo.swap_block_size = cfg.swap_block_size
// pthread_t hilo_km;

// if (pthread_create(&hilo_km, NULL, receptor_km, (void *)&parametros_hilo) != 0)
// {
//     // t_paquete paquete_tamanio = creat
//     perror("Error al crear hilo storage");
//     exit(EXIT_FAILURE);
// }
// pthread_join(hilo_km, NULL);
    
    return 0;
}
