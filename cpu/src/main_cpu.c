#include "main_cpu.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] [Identificador]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    int identificador = atoi(argv[2]);

    if (identificador <= 0)
    {
        fprintf(stderr, "Error: El identificador '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    cpu_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_cpu();

    //! CPU->KERNEL_MEMORY
    parametros_km.conexion_ks = crear_conexion(cpu_config.ip_kernel_memory, cpu_config.puerto_kernel_memory);
    if (parametros_km.conexion_ks == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        return -1;
    }
    enviar_operacion(parametros_km.conexion_ks, HANDSHAKE_CPU); // HANDSHAKE CPU->KM

    //! CPU->KERNEL_SCHEDULER
    parametros_km.conexion_km = crear_conexion(cpu_config.ip_kernel_scheduler, cpu_config.puerto_kernel_scheduler);
    if (parametros_km.conexion_km == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KS.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        return -1;
    }
    enviar_operacion(parametros_km.conexion_km, HANDSHAKE_CPU); // HANDSHAKE CPU->KS

    //! CPU->MEMORY_STICK
    parametros_km.conexion_ms = crear_conexion(cpu_config.ip_memory_stick, cpu_config.puerto_memory_stick);
    if (parametros_km.conexion_ms == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->MS.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        return -1;
    }
    enviar_operacion(parametros_km.conexion_ms, HANDSHAKE_CPU); // HANDSHAKE CPU->MS

    pthread_t hilo_km, hilo_ks, hilo_ms;
    if (pthread_create(&hilo_km, NULL, receptor_km, NULL) != 0)
    {
        perror("Error al crear hilo km");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&hilo_ks, NULL, receptor_ks, NULL) != 0)
    {
        perror("Error al crear hilo ks");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&hilo_ms, NULL, receptor_ms, NULL) != 0)
    {
        perror("Error al crear hilo ms");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo_km, NULL);
    pthread_join(hilo_ks, NULL);
    pthread_join(hilo_ms, NULL);

    log_debug(cpu_logger, "Memory Stick TERMINO.\n");

    cpu_liberar_config();
    cpu_liberar_log();

    return 0;
}
