#include "main_cpu.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] [Identificador_cpu]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    identificador_cpu = atoi(argv[2]);

    if (identificador_cpu <= 0)
    {
        fprintf(stderr, "Error: El identificador_cpu '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    cpu_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_cpu();
    inicializar_semaforos_coordinadores();
    creacion_conexiones_cpu();

    while (1)
    {
        // 1. Esperamos a que el receptor_ks nos avise que hay un proceso
        sem_wait(&sem_proceso_nuevo);
        flag_syscall = 0;

        // 2. Solicitud Contexto del CPU al KM
        t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
        agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
        enviar_paquete(paquete_solicitud_contexto, conexion_km);
        eliminar_paquete(paquete_solicitud_contexto);

        sem_wait(&finalizacion_procesamiento);
    }

    log_debug(cpu_logger, "CPU TERMINO.\n");

    liberar_todo();
    return 0;
}
