#include "ks_main.h"
#include "ks_maneja_cpu.h"
#include "ks_maneja_io.h"

static algoritmos ks_algoritmo_from_string(char *str)
{
    if (strcmp(str, "FIFO") == 0)
        return FIFO;
    if (strcmp(str, "RR") == 0)
        return RR;
    if (strcmp(str, "CMN") == 0)
        return CMN;

    return INVALIDO;
}

void *ks_parsear_algoritmos(char *raw)
{
    if (!raw)
    {
        cantidad_algoritmos = 0;
        array_queues = NULL;
        return;
    }

    // saco corchetes
    char *limpio = string_duplicate(raw);
    string_replace(&limpio, "[", "");
    string_replace(&limpio, "]", "");

    // separo por coma
    char **tokens = string_split(limpio, ",");

    // contar algoritmos
    cantidad_algoritmos = 0;
    while (tokens[cantidad_algoritmos] != NULL)
    {
        cantidad_algoritmos++;
    }

    // reservar memoria para array global
    array_queues = malloc(sizeof(t_elemento_array_algoritmos) * cantidad_algoritmos);

    // llenar array
    for (int i = 0; i < cantidad_algoritmos; i++)
    {
        string_trim(&tokens[i]);
        array_queues[i].alg = ks_algoritmo_from_string(tokens[i]);

        // crear queue vacía asociada
        array_queues[i].queue = queue_create();
    }

    // liberar auxiliares
    string_array_destroy(tokens);
    free(limpio);
}

bool ks_cargar_config(char *path_config, char* path_proceso_inicial)
{
    t_config *config = config_create(path_config);

    if (!config)
    {
        return false;
    }

    ks_config.ip_km = strdup(config_get_string_value(config, "IP_KM"));
    ks_config.puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
    ks_config.puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    ks_config.algoritmo = ks_algoritmo_from_string(config_get_string_value(config, "PLANIFICATION_ALGORITHM"));
    ks_config.algoritmo_queues = ks_parsear_algoritmos(config_get_string_value(config, "QUEUES_ALGORITHMS"));
    ks_config.rr_quantum = config_get_int_value(config, "RR_QUANTUM");
    config_get_string_value(config, "QUEUE_PREEMPTION");
    // ks_config.queue_preemption = strdup(config_get_string_value(config, "QUEUE_PREEMPTION"));
    if (strdup(config_get_string_value(config, "QUEUE_PREEMPTION")) == "TRUE")
    {
        ks_config.queue_preemption = true;
    }
    else
    {
        ks_config.queue_preemption = false;
    }

    ks_config.suspension_timeout = config_get_int_value(config, "SUSPENSION_TIMEOUT");
    ks_config.path_proceso_inicial = strdup(path_proceso_inicial);
    const char *log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level level = log_level_from_string((char *)log_level_string);

    ks_logger = log_create("kernel_scheduler.log", "KERNEL_SCHEDULER", 1, level);

    config_destroy(config);

    // // Leer la lista de algoritmo queues
    // for (int i = 0; i < list_size(ks_config.algoritmo_queues); i++) {
    // algoritmos* alg = list_get(ks_config.algoritmo_queues, i);

    //     switch (*alg) {
    //         case FIFO:
    //             printf("FIFO\n");
    //             break;
    //         case RR:
    //             printf("RR\n");
    //             break;
    //     }
    // }

    return ks_logger != NULL;
}

void ks_liberar_config(void)
{

    if (ks_logger)
    {
        log_destroy(ks_logger);
    }
}

void ks_liberar_array_queues(void)
{
    if (array_queues)
    {
        for (int i = 0; i < cantidad_algoritmos; i++)
        {
            queue_destroy(array_queues[i].queue);
        }

        free(array_queues);
    }
}
t_person* find_by_name_contains(t_list* people, char* substring) {
    bool _name_contains(void* ptr) {
        t_person* person = (t_person*) ptr;
        return string_contains(person->name, substring);
    }
    return list_find(people, _name_contains);
}
t_pcb* buscar_pcb_por_pid(int pid) {
    for (int i = 0; i < cantidad_algoritmos; i++) {

        bool _pcb_tiene_pid(void* ptr) {
            t_pcb* pcb = (t_pcb*) ptr;
            return pcb->id == pid;
        }

        t_pcb* encontrado = list_find(array_queues[i].queue, _pcb_tiene_pid);

        if (encontrado != NULL) {
            return encontrado;
        }
    }
    return NULL; // No se encontró el PCB con el PID dado
}

void *ks_conexion_loop(void *_arg)
{
    (void)_arg;

    printf("Entra en ks_conexion_loop");

    for (;;)
    {
        int fd = esperar_cliente(ks_server_fd);
        if (fd < 0)
        {
            log_error(ks_logger, "accept falló");
            continue;
        }

        log_info(ks_logger, "Se conectó un cliente en el socket: %d", fd);

        int op = recibir_operacion(fd);
        if (op <= 0)
        {
            close(fd);
            continue;
        }

        if (op == HANDSHAKE_IO)
        {
            log_info(ks_logger, "Se conecto un nuevo IO");
            // NO recibir_paquete aca
            // enviar_ks_ok_conexion(fd);
            pthread_t hilo_io;

            int *socket_ptr = malloc(sizeof(int));
            if (socket_ptr == NULL)
            {
                close(fd);
                return;
            }
            *socket_ptr = fd;

            if (pthread_create(&hilo_io, NULL, ks_maneja_io, socket_ptr) != 0)
            {
                log_error(ks_logger, "no se inicia el hilo cpu");
                free(socket_ptr);
                close(fd);
                return;
            }
            pthread_detach(hilo_io);
            log_info(ks_logger, "Se detachea el hilo_io");
        }
        else if (op == HANDSHAKE_CPU)
        {
            log_info(ks_logger, "Se conecto un nuevo CPU");
            // NO recibir_paquete aca
            pthread_t hilo_cpu;

            int *socket_ptr = malloc(sizeof(int));
            if (socket_ptr == NULL)
            {
                close(fd);
                return;
            }
            *socket_ptr = fd;

            if (pthread_create(&hilo_cpu, NULL, ks_maneja_cpu, socket_ptr) != 0)
            {
                log_error(ks_logger, "no se inicia el hilo cpu");
                free(socket_ptr);
                close(fd);
                return;
            }
            pthread_detach(hilo_cpu);
            log_info(ks_logger, "Se detachea el hilo_cpu");
        }

        else
        {
            log_error(ks_logger, "Handshake desconocido (%d)", op);
            // Si vino algo, descartalo
            t_paquete *trash = recibir_paquete(fd);
            if (trash)
                eliminar_paquete(trash);
            close(fd);
            continue;
        }
    }

    return NULL;
}

void *ks_conexion_km(void *_arg)
{
    (void)_arg;

    int socket_km = crear_conexion(ks_config.ip_km, ks_config.puerto_km);

    if (socket_km < 0)
    {
        log_error(ks_logger, "No pude conectar a Kernel Memory %s:%s", ks_config.ip_km, ks_config.puerto_km);
        return NULL;
    }

    log_info(ks_logger, "Conectado a Kernel Memory en %s:%s", ks_config.ip_km, ks_config.puerto_km);

    // HANDSHAKE
    enviar_operacion(socket_km, HANDSHAKE_KS);

    int op = recibir_operacion(socket_km);
    if (op <= 0)
    {
        log_error(ks_logger, "Kernel Memory cerró durante handshake");
        close(socket_km);
        return NULL;
    }

    if (op != KM_OK_CONEXION)
    {
        log_error(ks_logger, "Handshake invalido. Esperaba KM_OK_CONEXION (%d), llegó %d", KM_OK_CONEXION, op);
        close(socket_km);
        return NULL;
    }

    log_info(ks_logger, "Handshake con Kernel Memory OK");

    // Loop de comunicación con KM
    while (1)
    {
        int cod_op = recibir_operacion(socket_km);

        if (cod_op <= 0)
        {
            log_error(ks_logger, "Kernel Memory se desconectó");
            close(socket_km);
            break;
        }

        switch (cod_op)
        {
            // case RESPUESTA_MEMORIA:
            //     ...
            //     break;

        default:
            log_warning(ks_logger, "Operacion desconocida de KM (%d)", cod_op);
            t_paquete *basura = recibir_paquete(socket_km);
            if (basura)
                eliminar_paquete(basura);
            break;
        }
    }

    return NULL;
}

static void sigint_handler(int _sig)
{
    (void)_sig;

    if (ks_logger)
        log_info(ks_logger, "SIGINT: cerrando Kernel Scheduler...");

    if (ks_server_fd != -1)
        close(ks_server_fd);

    ks_liberar_config();

    exit(0);
}

int main(int argc, char* argv[]) //  ~ ./bin/kernel_scheduler [Archivo Config] [Path Proceso Inicial]
{

    if (!ks_cargar_config(argv[1], argv[2])) //! Está hardcodeado el codigo del config
    { 
        fprintf(stderr, "No pude cargar ks.config (busca en el CWD).\n");

        return EXIT_FAILURE;
    }
    log_debug(ks_logger, "Empezando la conexion KS");

    signal(SIGINT, sigint_handler);

    pthread_t hilo_km;
    pthread_create(&hilo_km, NULL, ks_conexion_km, NULL);
    pthread_detach(hilo_km);

    // ABRIR SERVIDOR DE ESCUCHA PARA CPU E IO
    ks_server_fd = iniciar_servidor(ks_config.puerto_escucha);
    if (ks_server_fd < 0)
    {
        log_error(ks_logger, "No pude iniciar servidor en %s", ks_config.puerto_escucha);
        return EXIT_FAILURE;
    }

    log_info(ks_logger, "Kernel Scheduler escuchando en puerto %s", ks_config.puerto_escucha);

    pthread_t hilo_conexiones;
    pthread_create(&hilo_conexiones, NULL, ks_conexion_loop, NULL);
    pthread_detach(hilo_conexiones);

    // dormir el main para siempre
    for (;;)
        pause();

    return 0;
}
