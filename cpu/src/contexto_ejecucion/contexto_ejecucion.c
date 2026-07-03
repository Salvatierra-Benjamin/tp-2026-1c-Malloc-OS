#include "contexto_ejecucion.h"

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

    buffer_read(&TAM_MAX_SEGMENTO, buffer, sizeof(int));
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

        for (int i = 0; i < contexto_actual->tabla_segmentos.cantidad_segmentos; i++)
        {
            log_info(cpu_logger, "Intentando leer los datos del Segmento: %d ...", i);
            log_info(cpu_logger, "Segmento %d -> Base: %u | Tamaño: %u", i,
                     contexto_actual->tabla_segmentos.segmento[i].dir_base,
                     contexto_actual->tabla_segmentos.segmento[i].tam_seg);
        }
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