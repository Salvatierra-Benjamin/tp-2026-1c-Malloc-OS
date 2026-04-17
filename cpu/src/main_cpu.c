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

    int conexion_cpu = crear_conexion(cpu_config.ip_km, cpu_config.puerto_km);
    if (conexion_cpu == -1)
    {
        log_info(cpu_logger, "ERROR AL CREAR CONEXION CPU->KM.\n");
        cpu_liberar_config();
        cpu_liberar_log();
        return -1;
    }
    enviar_operacion(conexion_cpu, HANDSHAKE_CPU); // Nuestro Handshake con KM

    int socket_servidor = iniciar_servidor(cpu_config.puerto_escucha_cpu);
    esperar_cpus(socket_servidor);

    log_debug(cpu_logger, "Memory Stick TERMINO.\n");

    cpu_liberar_config();
    cpu_liberar_log();

    return 0;
}
