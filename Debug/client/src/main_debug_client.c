#include "main_debug_client.h"

void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer)
{
    // 1. Leer datos fijos (PID y Registros)
    buffer_read(&(contexto->PID), buffer, sizeof(int));
    buffer_read(&(contexto->registros), buffer, sizeof(t_registros));

    // 2. Leer cantidad de segmentos (es vital para el malloc)
    buffer_read(&(contexto->tabla_segmentos.cantidad_segmentos), buffer, sizeof(uint32_t));

    // 3. Malloc para la tabla completa
    size_t tamano_tabla = sizeof(segmento_t) * contexto->tabla_segmentos.cantidad_segmentos;
    contexto->tabla_segmentos.segmento = malloc(tamano_tabla);

    // 4. Leer los datos de la tabla (el array completo)
    buffer_read(contexto->tabla_segmentos.segmento, buffer, tamano_tabla);
}

void Loggear_Contexto(t_contexto_ejecucion *contexto_actual)
{
    // =========================================================================
    //  LOGS PARA DEMOSTRAR LA DESERIALIZACIÓN
    // =========================================================================
    log_info(debug_client_logger, "================ CONTEXTO RECIBIDO ================");
    log_info(debug_client_logger, "PID: %d", contexto_actual->PID);

    log_info(debug_client_logger, "Registros CPU -> PC: %u | AX: %u | BX: %u | CX: %u | DX: %u",
             contexto_actual->registros.PC, contexto_actual->registros.AX,
             contexto_actual->registros.BX, contexto_actual->registros.CX,
             contexto_actual->registros.DX);

    log_info(debug_client_logger, "Registros EXT -> EAX: %u | EBX: %u | ECX: %u | EDX: %u",
             contexto_actual->registros.EAX, contexto_actual->registros.EBX,
             contexto_actual->registros.ECX, contexto_actual->registros.EDX);

    log_info(debug_client_logger, "Registros IDX -> SI: %u | DI: %u",
             contexto_actual->registros.SI, contexto_actual->registros.DI);

    log_info(debug_client_logger, "Cantidad de Segmentos recibidos: %d", contexto_actual->tabla_segmentos.cantidad_segmentos);

    // Acá está la prueba del delito: Mostramos el puntero falopa
    log_warning(debug_client_logger, "Dirección de memoria del puntero 'segmento': %p", (void *)contexto_actual->tabla_segmentos.segmento);

    // ESTO ES LO QUE VA A HACER EXPLOTAR EL PROGRAMA CON LA "FUNCIÓN MALA"
    if (contexto_actual->tabla_segmentos.cantidad_segmentos > 0)
    {
        log_info(debug_client_logger, "Intentando leer los datos del Segmento 0...");

        // Si el puntero es basura (forma mala), el programa muere en la siguiente línea:
        log_info(debug_client_logger, "Segmento 0 -> Base: %d | Tamaño: %d",
                 contexto_actual->tabla_segmentos.segmento[0].dir_base,
                 contexto_actual->tabla_segmentos.segmento[0].tam_seg);

        log_info(debug_client_logger, "Segmento 1 -> Base: %d | Tamaño: %d",
                 contexto_actual->tabla_segmentos.segmento[1].dir_base,
                 contexto_actual->tabla_segmentos.segmento[1].tam_seg);
    }
    log_info(debug_client_logger, "===================================================");
}

void Destruir_Contexto(t_contexto_ejecucion *contexto)
{
    if (contexto->tabla_segmentos.segmento != NULL)
    {
        free(contexto->tabla_segmentos.segmento);
        contexto->tabla_segmentos.segmento = NULL;
    }
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "ERROR: ARGUMENTOS INSUFICIENTES\n");
        fprintf(stderr, "Uso: %s [path_config] [Identificador_debug_client]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    identificador_debug_client = atoi(argv[2]);

    if (identificador_debug_client <= 0)
    {
        fprintf(stderr, "Error: El identificador_debug_client '%s' no es válido (Debe ser un entero > 0)\n", argv[2]);
        return EXIT_FAILURE;
    }

    debug_client_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config_debug_client();
    inicializar_semaforos_coordinadores();
    creacion_conexiones_debug_client();

    //! HANDSHAKE DINAMICO CON MS
    while (1)
    {
        // 1. Esperamos a que el receptor_ks nos avise que hay un proceso
        int respuesta_alojar_proceso = recibir_operacion(conexion_debug_server);
        if (respuesta_alojar_proceso == ALOJAR_PROCESO)
        {
            t_paquete *paquete_alojar_proceso = recibir_paquete(conexion_debug_server);
            buffer_read(&PID_a_ejecutar, paquete_alojar_proceso->buffer, sizeof(int));
            eliminar_paquete(paquete_alojar_proceso);
        }

        // 2. Solicitud Contexto del debug_client al KS
        t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
        agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
        enviar_paquete(paquete_solicitud_contexto, conexion_debug_server);
        eliminar_paquete(paquete_solicitud_contexto);

        int respuesta_contexto_ejecucion = recibir_operacion(conexion_debug_server);
        if (respuesta_contexto_ejecucion == CONTEXTO_EJECUCION)
        {
            t_paquete *paquete_contexto = recibir_paquete(conexion_debug_server);

            t_contexto_ejecucion contexto_actual;
            Recibir_Contexto(&contexto_actual, paquete_contexto->buffer);
            // buffer_read(&contexto_actual, paquete_contexto->buffer, sizeof(contexto_actual));
            // eliminar_paquete(paquete_contexto);
            Loggear_Contexto(&contexto_actual);

            // 3. Acá arranca el ciclo (Fetch -> Decode -> Execute)
            log_info(debug_client_logger, "Iniciando ciclo para el PID: %d", PID_a_ejecutar);
            Ejecutar_Ciclo_de_Instruccion(&contexto_actual);
            Destruir_Contexto(&contexto_actual); // USAR EN RECIBIR CONTEXTO
        }

        // Cuando la función terminar, el while vuelve arriba y se vuelve a bloquear en el sem_wait esperando el próximo proceso.
    }

    log_debug(debug_client_logger, "debug_client TERMINO.\n");

    liberar_todo();
    return 0;
}
