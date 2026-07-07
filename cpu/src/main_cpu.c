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
        flag_seg_fault = 0;

        // 2. Solicitud Contexto del CPU al KM
        t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
        agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
        enviar_paquete(paquete_solicitud_contexto, conexion_km);
        eliminar_paquete(paquete_solicitud_contexto);

        sem_wait(&sem_contexto_recibido);

        // PASOS PARA PASAR EL CONTEXTO CON EL AUXILIAR :
        t_contexto_ejecucion contexto_actual;

        // PASO A: Copia superficial de los datos fijos (PID, registros y cantidad_segmentos)
        contexto_actual = contexto_aux_global;

        // PASO B: Reservar memoria propia para los segmentos locales de la CPU
        size_t tamano_tabla = sizeof(segmento_t) * contexto_actual.tabla_segmentos.cantidad_segmentos;
        contexto_actual.tabla_segmentos.segmento = malloc(tamano_tabla);

        // PASO C: Copiar los datos reales de la tabla global a la tabla local
        memcpy(contexto_actual.tabla_segmentos.segmento, contexto_aux_global.tabla_segmentos.segmento, tamano_tabla);

        sem_post(&sem_contexto_procesado);

        //  3. Acá arranca el ciclo (Fetch -> Decode -> Execute)
        log_info(cpu_logger, "Iniciando ciclo para el PID: %d", PID_a_ejecutar);
        Ejecutar_Ciclo_de_Instruccion(&contexto_actual);

        Destruir_Contexto(&contexto_actual); // USAR EN RECIBIR CONTEXTO
    }

    log_debug(cpu_logger, "CPU TERMINO.\n");

    liberar_todo();
    return 0;
}
