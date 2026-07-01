#include "decode.h"
// comentario borrar
t_executor tabla_comandos[] = {
    [NOOP] = execute_NOOP,
    [SET] = execute_SET,
    // [MOV_IN]       = execute_MOV_IN,
    // [MOV_OUT]      = execute_MOV_OUT,
    [SUM] = execute_SUM,
    [SUB] = execute_SUB,
    [JNZ] = execute_JNZ,
    // [COPY_MEM]     = execute_COPY_MEM,

    // --- SYSCALLS --- //
    [MUTEX_CREATE] = execute_MUTEX_CREATE,
    [MUTEX_LOCK] = execute_MUTEX_LOCK,
    [MUTEX_UNLOCK] = execute_MUTEX_UNLOCK,
    [MEM_ALLOC] = execute_MEM_ALLOC,
    [MEM_FREE] = execute_MEM_FREE,
    [SLEEP] = execute_SLEEP,
    [STDOUT] = execute_STDOUT,
    [STDIN] = execute_STDIN,
    [INIT_PROC] = execute_INIT_PROC,
    [EXIT] = execute_EXIT};

void Iniciar_Decode(t_contexto_ejecucion *contexto_actual, t_instruccion instruccion)
{
    t_palabra_reservada comando = instruccion.comando;
    char *parametro_1 = instruccion.parametro_1;
    char *parametro_2 = instruccion.parametro_2;

    if (comando >= 0 && tabla_comandos[comando] != NULL)
    {
        // Llamada directa en O(1) pasándole todo de una
        tabla_comandos[comando](contexto_actual, parametro_1, parametro_2);
    }
    else
    {
        log_error(cpu_logger, "Error: Instruccion recibida no recibida o no implementada.");
        log_error(cpu_logger, "Codigo de instruccion recibida: %d", comando);
    }
}