#include "fetch.h"

// Esta función solo arma el paquete y lo manda
void Solicitar_Instruccion(int PID, int PC)
{
    t_paquete *paquete = crear_paquete(SOLICITUD_INSTRUCCION);
    agregar_a_paquete(paquete, &PID, sizeof(int));
    agregar_a_paquete(paquete, &PC, sizeof(int));
    enviar_paquete(paquete, conexion_km);
    eliminar_paquete(paquete);
}

void Recibir_Instruccion(t_instruccion *nueva_instruccion)
{
    int respuesta = recibir_operacion(conexion_km);
    if (respuesta == ENVIO_INSTRUCCION)
    {

        t_paquete *paquete = recibir_paquete(conexion_km);

        // 1. Deserializar lo de tamaño fijo (comando y PID)
        buffer_read(&(nueva_instruccion->comando), paquete->buffer, sizeof(t_palabra_reservada));
        buffer_read(&(nueva_instruccion->PID), paquete->buffer, sizeof(int));

        // 2. Deserializar parametro_1 (Dinámico)
        int len1;
        buffer_read(&len1, paquete->buffer, sizeof(int));                   // Leemos cuánto mide
        nueva_instruccion->parametro_1 = malloc(len1);                      // Pedimos el espacio justo
        buffer_read(nueva_instruccion->parametro_1, paquete->buffer, len1); // Copiamos el contenido

        // 3. Deserializar parametro_2 (Dinámico)
        int len2;
        buffer_read(&len2, paquete->buffer, sizeof(int));
        nueva_instruccion->parametro_2 = malloc(len2);
        buffer_read(nueva_instruccion->parametro_2, paquete->buffer, len2);

        eliminar_paquete(paquete);
    }
    else
    {
        log_error(cpu_logger, "Instruccion no recibida");
        exit(EXIT_FAILURE);
    }
}

void Loggear_Instruccion(t_instruccion *instruccion)
{
    // =========================================================================
    //  LOGS DE CONTROL: DESERIALIZACIÓN DE INSTRUCCIÓN
    // =========================================================================
    log_info(cpu_logger, "================ INSTRUCCIÓN RECIBIDA ================");

    // Suponiendo que 'comando' es un enum o entero, lo imprimimos como %d o %u
    log_info(cpu_logger, "ID Comando: %d | PID Asociado: %d",
             instruccion->comando, instruccion->PID);

    // Verificamos e imprimimos el Parámetro 1 (Dinámico)
    if (instruccion->parametro_1 != NULL)
    {
        log_info(cpu_logger, "Parámetro 1 [Dirección: %p]: \"%s\"",
                 (void *)instruccion->parametro_1, instruccion->parametro_1);
    }
    else
    {
        log_warning(cpu_logger, "Parámetro 1 es NULL");
    }

    // Verificamos e imprimimos el Parámetro 2 (Dinámico)
    if (instruccion->parametro_2 != NULL)
    {
        log_info(cpu_logger, "Parámetro 2 [Dirección: %p]: \"%s\"",
                 (void *)instruccion->parametro_2, instruccion->parametro_2);
    }
    else
    {
        log_warning(cpu_logger, "Parámetro 2 es NULL");
    }

    log_info(cpu_logger, "======================================================");
}

void Destruir_Instruccion(t_instruccion *instruccion)
{
    if (instruccion->parametro_1 != NULL)
        free(instruccion->parametro_1); // ¡OBLIGATORIO!
    if (instruccion->parametro_2 != NULL)
        free(instruccion->parametro_2); // ¡OBLIGATORIO!

    // Opcional: limpiar los punteros para no dejar basura
    instruccion->parametro_1 = NULL;
    instruccion->parametro_2 = NULL;
}

void Iniciar_Fetch(t_contexto_ejecucion *contexto_actual, t_instruccion *nueva_instruccion)
{
    Solicitar_Instruccion(contexto_actual->PID, contexto_actual->registros.PC);
    Recibir_Instruccion(nueva_instruccion);
    Loggear_Instruccion(nueva_instruccion);
}
