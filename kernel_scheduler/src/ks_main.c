#include <ks_main.h>
#include "unistd.h"

t_log* ks_logger = NULL;
ks_config  ks_config;
int ks_server_fd = -1;

bool ks_cargar_config(char* path){
    t_config* config = config_create(path);

    if(!config) {
        return false;
    }

    ks_config.ip_kernel_memory = strdup(config_get_string_value(config,"IP_KERNEL_MEMORY"));
    ks_config.puerto_escucha = strdup(config_get_string_value(config,"PUERTO_ESCUCHA"));
    ks_config.algoritmo = strdup(config_get_string_value(config,"PLANIFICATION_ALGORITHM"));  // FIFO, RR, CMN
    ks_config.algoritmo_queues = strdup(config_get_string_value(config,"QUEUES_ALGORITHMS")); // lista entre FIFO, RR
    ks_config.rr_quantum = config_get_int_value(config,"RR_QUANTUM");
    ks_config.queue_preemption = config_get_bool_value(config,"QUEUE_PREEMPTION");
    ks_config.suspension_timeout = config_get_int_value(config,"SUSPENSION_TIMEOUT");
    const char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level level = log_level_from_string((char*)log_level_string);

    ks_logger = log_create("kernel_scheduler.log","KERNEL_SCHEDULER",1, level);

    config_destroy(config);
    
    return ks_logger != NULL;
}

void ks_liberar_config(void){
    free(ks_config.puerto_escucha);
    free(ks_config.algoritmo);
    free(ks_config.algoritmo_queues);

    if(ks_logger) {
        log_destroy(ks_logger);
    }
}

void* ks_conexion_loop(void* _arg){
    (void)_arg;

    for(;;){
        int fd = esperar_cliente(ks_server_fd);
        if(fd < 0) { 
            log_error(ks_logger,"accept falló"); 
            continue; 
        }

        log_info(ks_logger,"Se conectó un cliente en el socket: %d", fd);

        int op = recibir_operacion(fd);
        if(op <= 0){ 
            close(fd); 
            continue; 
        }

        pthread_t hilo;
        if (op == HANDSHAKE_IO) {
            log_info(ks_logger,"Se conecto un nuevo IO");
            // NO recibir_paquete aca
            enviar_ks_ok_conexion(fd);
            pthread_create(&hilo,NULL,(void*(*)(void*))ks_maneja_io,(void*)(intptr_t)fd);
            pthread_detach(hilo);

        } else if (op == HANDSHAKE_CPU) {
            log_info(ks_logger,"Se conecto un nuevo CPU");
            // NO recibir_paquete aca
            pthread_create(&hilo,NULL,(void*(*)(void*))ks_maneja_cpu,(void*)(intptr_t)fd);
            pthread_detach(hilo);
        }

        else {
            log_error(ks_logger,"Handshake desconocido (%d)", op);
            // Si vino algo, descartalo
            t_paquete* trash = recibir_paquete(fd);
            if(trash) eliminar_paquete(trash);
            close(fd);
            continue;
        }
    }
    return NULL;
}

void* ks_conexion_km(void* _arg){
    (void)_arg;

    int socket_km = crear_conexion(
        ks_config.ip_kernel_memory, 
        ks_config.puerto_escucha
    );

    if(socket_km < 0){
        log_error(ks_logger, 
            "No pude conectar a Kernel Memory %s:%s", 
            ks_config.ip_kernel_memory, 
            ks_config.puerto_escucha
        );
        return NULL;
    }

    log_info(ks_logger, 
        "Conectado a Kernel Memory en %s:%s", 
        ks_config.ip_kernel_memory, 
        ks_config.puerto_escucha
    );

    // HANDSHAKE
    enviar_operacion(socket_km, HANDSHAKE_KS);

    int op = recibir_operacion(socket_km);
    if(op <= 0){
        log_error(ks_logger, "Kernel Memory cerró durante handshake");
        close(socket_km);
        return NULL;
    }

    if(op != KM_OK_CONEXION){
        log_error(ks_logger, 
            "Handshake invalido. Esperaba KM_OK_CONEXION (%d), llegó %d", 
            KM_OK_CONEXION, op
        );
        close(socket_km);
        return NULL;
    }

    log_info(ks_logger, "Handshake con Kernel Memory OK");

    // 🔁 Loop de comunicación con KM
    while(1){
        int cod_op = recibir_operacion(socket_km);

        if(cod_op <= 0){
            log_error(ks_logger, "Kernel Memory se desconectó");
            close(socket_km);
            break;
        }

        switch(cod_op){
            // case RESPUESTA_MEMORIA:
            //     ...
            //     break;

            default:
                log_warning(ks_logger, "Operacion desconocida de KM (%d)", cod_op);
                t_paquete* basura = recibir_paquete(socket_km);
                if(basura) eliminar_paquete(basura);
                break;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if(!ks_cargar_config("master.config")){
        fprintf(stderr, "No pude cargar kernel_scheduler.config (busca en el CWD).\n");

        return EXIT_FAILURE;
    }
    
    signal(SIGINT, sigint_handler);

    pthread_t hilo_km;
    pthread_create(&hilo_km, NULL, ks_conexion_km, NULL);
    pthread_detach(hilo_km);

    // ABRIR SERVIDOR DE ESCUCHA PARA CPU E IO
    ks_server_fd = iniciar_servidor(ks_config.puerto_escucha);
    if(ks_server_fd < 0) { 
        log_error(ks_logger,"No pude iniciar servidor en %s", ks_config.puerto_escucha); 
        return EXIT_FAILURE; 
    }

    log_info(ks_logger,"Kernel Scheduler escuchando en puerto %s", ks_config.puerto_escucha);  

    pthread_t hilo_conexiones; //, hilo_io;
    pthread_create(&hilo_conexiones,  NULL, ks_conexion_loop, NULL);
    // pthread_create(&hilo_io, NULL, ks_io_loop, NULL);
    // pthread_detach(hilo_io);
    pthread_detach(hilo_conexiones);

    // dormir el main para siempre
    for(;;) pause();

    return 0;
}
