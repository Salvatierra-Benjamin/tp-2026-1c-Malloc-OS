#include "fetch.h"

void Iniciar_Fetch(t_contexto_ejecucion *contexto_actual, t_instruccion *nueva_instruccion)
{
    Solicitar_Instruccion(contexto_actual->PID, contexto_actual->registros.PC);
    Recibir_Instruccion(nueva_instruccion);
    Loggear_Instruccion(nueva_instruccion);
    log_info(cpu_logger, "## PID: %d - FETCH - Program Counter: %u", contexto_actual->PID, contexto_actual->registros.PC);
}
