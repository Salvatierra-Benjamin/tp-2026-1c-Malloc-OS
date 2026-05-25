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
        buffer_read(&len1, paquete->buffer, sizeof(int)); // Leemos cuánto mide
        // Si len1 es 0 (o menor/igual a 1 si mandan un string vacío ""), no hacemos malloc
        if (len1 <= 1)
        { // Si el tamaño es 0 no hay nada que limpiar en el buffer, por lo tanto lo setea en NULL.
            nueva_instruccion->parametro_1 = NULL;
            log_warning(cpu_logger, "[CPU] Parámetro 1 no provisto (len=%d). Seteado en NULL.", len1);
            // Si KM igual mandó 1 byte (el \0), avanzamos el buffer para no desfasarnos
            if (len1 == 1)
            {
                char basura;                              // Reservamos 1 byte de espacio temporal en la pila (stack)
                buffer_read(&basura, paquete->buffer, 1); // Sacamos ese byte molesto del buffer y lo metemos ahí
                log_warning(cpu_logger, "[CPU] Parámetro 1 vacío detectado (len=1). Se limpió el byte restante del buffer.");
                // Al salir de la función, la variable 'basura' desaparece y ese byte se desecha.
            }
        }
        else
        {
            nueva_instruccion->parametro_1 = malloc(len1);                      // Pedimos el espacio justo
            buffer_read(nueva_instruccion->parametro_1, paquete->buffer, len1); // Copiamos el contenido
        }

        // 3. Deserializar parametro_2 (Dinámico)
        int len2;
        buffer_read(&len2, paquete->buffer, sizeof(int));
        if (len2 <= 1)
        {
            nueva_instruccion->parametro_2 = NULL;
            log_warning(cpu_logger, "[CPU] Parámetro 2 no provisto (len=%d). Seteado en NULL.", len2);
            if (len2 == 1)
            {
                char basura;
                buffer_read(&basura, paquete->buffer, 1);
                log_warning(cpu_logger, "[CPU] Parámetro 2 vacío detectado (len=1). Se limpió el byte restante del buffer.");
            }
        }
        else
        {
            nueva_instruccion->parametro_2 = malloc(len2);
            buffer_read(nueva_instruccion->parametro_2, paquete->buffer, len2);
        }

        eliminar_paquete(paquete);
    }
    else if (respuesta == ERROR_ENVIO_INSTRUCCION)
    {
        log_error(cpu_logger, "[CPU] KM notificó un error al intentar obtener la instrucción (¿Fin de archivo sin EXIT?).");

        // Seteamos el comando en un estado de error para que el ciclo de instrucción sepa qué hacer.
        nueva_instruccion->comando = ERROR_INSTRUCCION;
        nueva_instruccion->parametro_1 = NULL;
        nueva_instruccion->parametro_2 = NULL;
        // Importante hay que ver que hago en el Ciclo de Instruccion.
        // NO SE LIBERA CON FREE () NADA !!! PUNTEROS APUNTANDO A NULL.
    }
    else
    {
        log_error(cpu_logger, "[CPU] Código de operación desconocido esperando instrucción: %d", respuesta);
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
    contexto_actual->registros.PC++;
}
/*
* Sumo aca el PC. En la proxima vuelta del while(1) buscara la siguientes instruccion.
* En caso de que se ejecute el JNZ, este PC++ será pisado por el nuevo PC

* De paso me ahorro repetir PC++ en las todos los execute_
*/
