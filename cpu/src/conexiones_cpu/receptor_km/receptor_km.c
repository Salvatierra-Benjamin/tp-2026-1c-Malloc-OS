#include "receptor_km.h"
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

    buffer_read(TAM_MAX_SEGMENTO, buffer, sizeof(int));
}

void Loggear_Contexto(t_contexto_ejecucion *contexto_actual)
{
    // =========================================================================
    //  LOGS PARA DEMOSTRAR LA DESERIALIZACIÓN
    // =========================================================================
    log_info(cpu_logger, "================ CONTEXTO RECIBIDO ================");
    log_info(cpu_logger, "PID: %d", contexto_actual->PID);

    log_info(cpu_logger, "Registros CPU -> PC: %u | AX: %u | BX: %u | CX: %u | DX: %u",
             contexto_actual->registros.PC, contexto_actual->registros.AX,
             contexto_actual->registros.BX, contexto_actual->registros.CX,
             contexto_actual->registros.DX);

    log_info(cpu_logger, "Registros EXT -> EAX: %u | EBX: %u | ECX: %u | EDX: %u",
             contexto_actual->registros.EAX, contexto_actual->registros.EBX,
             contexto_actual->registros.ECX, contexto_actual->registros.EDX);

    log_info(cpu_logger, "Registros IDX -> SI: %u | DI: %u",
             contexto_actual->registros.SI, contexto_actual->registros.DI);

    log_info(cpu_logger, "Cantidad de Segmentos recibidos: %d", contexto_actual->tabla_segmentos.cantidad_segmentos);

    // Acá está la prueba del delito: Mostramos el puntero falopa
    log_warning(cpu_logger, "Dirección de memoria del puntero 'segmento': %p", (void *)contexto_actual->tabla_segmentos.segmento);

    // ESTO ES LO QUE VA A HACER EXPLOTAR EL PROGRAMA CON LA "FUNCIÓN MALA"
    if (contexto_actual->tabla_segmentos.cantidad_segmentos > 0)
    {
        log_info(cpu_logger, "Intentando leer los datos del Segmento 0...");

        // Si el puntero es basura (forma mala), el programa muere en la siguiente línea:
        log_info(cpu_logger, "Segmento 0 -> Base: %d | Tamaño: %d",
                 contexto_actual->tabla_segmentos.segmento[0].dir_base,
                 contexto_actual->tabla_segmentos.segmento[0].tam_seg);

        log_info(cpu_logger, "Segmento 1 -> Base: %d | Tamaño: %d",
                 contexto_actual->tabla_segmentos.segmento[1].dir_base,
                 contexto_actual->tabla_segmentos.segmento[1].tam_seg);
    }
    log_info(cpu_logger, "===================================================");
}

void Destruir_Contexto(t_contexto_ejecucion *contexto)
{
    // 1. Limpieza de memoria dinámica (Lo que ya tenías)
    if (contexto->tabla_segmentos.segmento != NULL)
    {
        free(contexto->tabla_segmentos.segmento);
        contexto->tabla_segmentos.segmento = NULL;
    }

    // 2. Limpieza de metadatos (Reset a 0)
    contexto->PID = -1;
    contexto->tabla_segmentos.cantidad_segmentos = 0;

    // 3. Reset total de registros (Opcional, pero recomendado por higiene)
    // Esto pone todos los bytes de la estructura de registros en 0
    memset(&(contexto->registros), 0, sizeof(t_registros));

    log_info(cpu_logger, "Contexto destruido y estructura limpiada.");
}

void receptor_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] Conectando con KS...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_detach(hilo_atencion_km);
}

void *atencion_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] CPU CONECTADO CON KS");

    while (1)
    {

        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case CONTEXTO_EJECUCION:
        {
            t_paquete *paquete_contexto = recibir_paquete(conexion_km);

            t_contexto_ejecucion contexto_actual;
            Recibir_Contexto(&contexto_actual, paquete_contexto->buffer);
            Loggear_Contexto(&contexto_actual);

            // 3. Acá arranca el ciclo (Fetch -> Decode -> Execute)
            log_info(cpu_logger, "Iniciando ciclo para el PID: %d", PID_a_ejecutar);
            Ejecutar_Ciclo_de_Instruccion(&contexto_actual);

            Destruir_Contexto(&contexto_actual); // USAR EN RECIBIR CONTEXTO

            sem_post(&finalizacion_procesamiento);
        }
        case ERROR_ENVIO_CONTEXTO:
        {
            log_error(cpu_logger, "[CPU] KM reportó que el contexto para PID %d no existe o es NULL.", PID_a_ejecutar);
            exit(EXIT_FAILURE);
            break;
        }
        case RECIBIR_MEMORY_STICK:
        {
            t_memory_stick *nuevo_ms = malloc(sizeof(t_memory_stick));
            t_paquete *recibir_MS = recibir_paquete(conexion_km);
            int size_ip, size_puerto;

            buffer_read(&(nuevo_ms->base), recibir_MS->buffer, sizeof(int));
            buffer_read(&(nuevo_ms->tamanio), recibir_MS->buffer, sizeof(int));

            // Desempaquetar IP
            buffer_read(&size_ip, recibir_MS->buffer, sizeof(int));
            nuevo_ms->ip = malloc(size_ip);
            buffer_read(nuevo_ms->ip, recibir_MS->buffer, size_ip);

            // Desempaquetar Puerto
            buffer_read(&size_puerto, recibir_MS->buffer, sizeof(int));
            nuevo_ms->puerto = malloc(size_puerto);
            buffer_read(nuevo_ms->puerto, recibir_MS->buffer, size_puerto);
            eliminar_paquete(recibir_MS);

            list_add(lista_memory_sticks, nuevo_ms);

            // --- CONEXIÓN AL MEMORY STICK ---
            conexion_ms = crear_conexion(nuevo_ms->ip, nuevo_ms->puerto);
            if (conexion_ms == -1)
            {
                log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->MS.");
                liberar_todo();
                exit(EXIT_FAILURE);
            }

            if (!realizar_handshake(conexion_ms, identificador_cpu, "Memory Stick"))
            {
                liberar_todo();
                exit(EXIT_FAILURE);
            }

            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KS_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
