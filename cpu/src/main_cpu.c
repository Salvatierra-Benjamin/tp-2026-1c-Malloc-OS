#include "main_cpu.h"
void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer)
{
    // 1. Leer datos fijos (PID y Registros)
    buffer_read(&(contexto->PID), buffer, sizeof(int));
    buffer_read(&(contexto->registros), buffer, sizeof(t_registros));

    // 2. Leer cantidad de segmentos (es vital para el malloc)
    buffer_read(&(contexto->tabla_segmentos.cantidad_segmentos), buffer, sizeof(uint32_t));

    // 3. Malloc para la tabla completa
    size_t tamano_tabla = sizeof(segmento_t) * contexto->tabla_segmentos.cantidad_segmentos;
    contexto->tabla_segmentos.tabla_segmentos = malloc(tamano_tabla);

    // 4. Leer los datos de la tabla (el array completo)
    buffer_read(contexto->tabla_segmentos.tabla_segmentos, buffer, tamano_tabla);
}
void destruir_contexto(t_contexto_ejecucion *contexto)
{
    if (contexto->tabla_segmentos.tabla_segmentos != NULL)
    {
        free(contexto->tabla_segmentos.tabla_segmentos);
        contexto->tabla_segmentos.tabla_segmentos = NULL;
    }
}
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

    sem_init(&sem_proceso_nuevo, 0, 0);

    cpu_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_cpu();
    inicializar_semaforos_coordinadores();
    creacion_conexiones_cpu();

    //! HANDSHAKE DINAMICO CON MS
    while (1)
    {
        // 1. Esperamos a que el receptor_ks nos avise que hay un proceso
        sem_wait(&sem_proceso_nuevo);

        // 2. Solicitud Contexto del CPU al KS
        t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
        agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
        enviar_paquete(paquete_solicitud_contexto, conexion_km);
        eliminar_paquete(paquete_solicitud_contexto);

        int respuesta = recibir_operacion(conexion_km);
        if (respuesta == CONTEXTO_EJECUCION)
        {
            t_paquete *paquete_contexto = recibir_paquete(conexion_km);

            t_contexto_ejecucion contexto_actual;
            buffer_read(&contexto_actual, paquete_contexto->buffer, sizeof(contexto_actual));
            eliminar_paquete(paquete_contexto);

            // 3. Acá arranca el ciclo (Fetch -> Decode -> Execute)
            log_info(cpu_logger, "Iniciando ciclo para el PID: %d", PID_a_ejecutar);
            Ejecutar_Ciclo_de_Instruccion(&contexto_actual);
        }

        // Cuando la función terminar, el while vuelve arriba y se vuelve a bloquear en el sem_wait esperando el próximo proceso.
    }

    log_debug(cpu_logger, "CPU TERMINO.\n");

    liberar_todo();
    return 0;
}
