#include "operaciones_instrucciones.h"

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
    // 1. Esperar a que el hilo receptor haya terminado de deserializar
    sem_wait(&sem_instruccion_recibida);

    // 2. Copia de campos primitivos (estos se copian valor a valor, sin peligro)
    nueva_instruccion->comando = instruccion_aux_global.comando;
    nueva_instruccion->PID = instruccion_aux_global.PID;

    // 3. Copia profunda (Deep Copy) de los parámetros dinámicos
    // A. Verificamos si existe contenido (!= NULL) para evitar Segmentation Fault.
    // B. Usamos strdup para reservar memoria propia y copiar el valor.
    // C. Si es NULL, asignamos NULL para mantener la integridad.
    nueva_instruccion->parametro_1 = (instruccion_aux_global.parametro_1 != NULL) ? strdup(instruccion_aux_global.parametro_1) : NULL;
    nueva_instruccion->parametro_2 = (instruccion_aux_global.parametro_2 != NULL) ? strdup(instruccion_aux_global.parametro_2) : NULL;

    // 4. Avisar al hilo receptor que ya copiamos todo y puede destruir la global
    sem_post(&sem_instruccion_procesada);
}

void Loggear_Instruccion(t_instruccion *instruccion)
{
    // Dump de deserialización de instrucción: solo diagnóstico, en log_debug para no
    // ensuciar el output obligatorio con LOG_LEVEL=INFO (el log "Ejecutando" del
    // enunciado lo emite execute_*).
    log_debug(cpu_logger, "================ INSTRUCCIÓN RECIBIDA ================");
    log_debug(cpu_logger, "ID Comando: %d | PID Asociado: %d",
              instruccion->comando, instruccion->PID);

    if (instruccion->parametro_1 != NULL)
        log_debug(cpu_logger, "Parámetro 1: \"%s\"", instruccion->parametro_1);
    else
        log_debug(cpu_logger, "Parámetro 1 es NULL");

    if (instruccion->parametro_2 != NULL)
        log_debug(cpu_logger, "Parámetro 2: \"%s\"", instruccion->parametro_2);
    else
        log_debug(cpu_logger, "Parámetro 2 es NULL");

    log_debug(cpu_logger, "======================================================");
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

    // 2. Resetear metadatos a valores inválidos/seguros
    instruccion->PID = -1;
    instruccion->comando = ERROR_INSTRUCCION; // Usamos el valor del enum que representa error

    // Con esto, la instrucción queda "muerta" y es imposible ejecutarla por error.
}