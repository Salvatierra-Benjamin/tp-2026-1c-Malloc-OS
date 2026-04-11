#include <sp_main.h>

int main(int argc, char* argv[]) {
   
    char* path = arg[1];

    swap_config* cfg = swap_cargar_config_log(path)

   


int conexion_sp = crear_conexion(cfg.ip_km , cfg.puerto_km);
enviar_operacion(conexion_sp, HANDSHAKE_SWAP);   // Nuestro Handshake Storage
    
 log_debug(sp_logger, "SWAP TERMINO");
    config_destroy(cfg);
    return g_logger != NULL;



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
