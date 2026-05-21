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

// Esta función recibe el stream y lo convierte a tu struct t_instruccion
void Recibir_Instruccion1(t_instruccion *nueva_intruccion)
{
    int respuesta = recibir_operacion(conexion_km);
    if (respuesta == ENVIO_INSTRUCCION)
    {
        t_instruccion aux;
        t_paquete *paquete = recibir_paquete(conexion_km); // Tu función de antes
        buffer_read(&aux, paquete->buffer, sizeof(aux));
        nueva_intruccion = aux;
        eliminar_paquete(paquete);
    }
    else
    {
        log_error(cpu_logger, "Instruccion no recibida");
        exit(EXIT_FAILURE);
    }
}

void Recibir_Instruccion2(t_instruccion *nueva_intruccion)
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

void destruir_instruccion(t_instruccion *instruccion)
{
    if (instruccion->parametro_1 != NULL)
        free(instruccion->parametro_1);
    if (instruccion->parametro_2 != NULL)
        free(instruccion->parametro_2);

    // Opcional: limpiar los punteros para no dejar basura
    instruccion->parametro_1 = NULL;
    instruccion->parametro_2 = NULL;
}

void Iniciar_Fetch(t_contexto_ejecucion *contexto_actual, t_instruccion *nueva_intruccion)
{
    Solicitar_Instruccion(contexto_actual->PID, contexto_actual->registros.PC);
    Recibir_Instruccion(nueva_intruccion);
    destruir_instruccion(nueva_intruccion);
}

// Ejemplo conceptual en KM (enviar)
// int len1 = strlen(instr.parametro_1) + 1; // +1 para el '\0'
// agregar_a_paquete(paquete, &len1, sizeof(int));
// agregar_a_paquete(paquete, instr.parametro_1, len1);
