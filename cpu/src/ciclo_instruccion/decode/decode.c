#include "decode.h"

void Iniciar_Decode(t_contexto_ejecucion *contexto_actual, t_instruccion instruccion)
{
    t_palabra_reservada comando = instruccion.comando;
    char *parametro_1 = instruccion.parametro_1;
    char *parametro_2 = instruccion.parametro_2;

    switch (comando)
    {
    case NOOP:
        execute_NOOP(contexto_actual);
        break;

    case SET:
        execute_SET(contexto_actual, parametro_1, parametro_2);
        break;

    case MOV_IN:
        // execute_MOV_IN(contexto_actual, parametro_1);
        break;

    case MOV_OUT:
        // execute_MOV_OUT(contexto_actual, parametro_1);
        break;

    case SUM:
        execute_SUM(contexto_actual, parametro_1, parametro_2);
        break;

    case SUB:
        execute_SUB(contexto_actual, parametro_1, parametro_2);
        break;

    case JNZ:
        execute_JNZ(contexto_actual, parametro_1, parametro_2);
        break;

    case COPY_MEM:
        // execute_COPY_MEM(contexto_actual, parametro_1);
        break;

        // SYCALLS
    case MUTEX_CREATE:
        // execute_MUTEX_CREATE(contexto_actual, parametro_1);
        break;

    case MUTEX_LOCK:
        // execute_MUTEX_LOCK(contexto_actual, parametro_1);
        break;

    case MUTEX_UNLOCK:
        // execute_MUTEX_UNLOCK(contexto_actual, parametro_1);
        break;

    case MEM_ALLOC:
        // execute_MEM_ALLOC(contexto_actual, parametro_1, parametro_2);
        break;

    case MEM_FREE:
        // execute_MEM_FREE(contexto_actual, parametro_1);
        break;

    case SLEEP:
        // execute_SLEEP(contexto_actual, parametro_1);
        break;

    case STDOUT:
        // execute_STDOUT(contexto_actual, parametro_1, parametro_2);
        break;

    case STDIN:
        // execute_STDIN(contexto_actual, parametro_1, parametro_2);
        break;

    case INIT_PROC:
        // execute_INIT_PROC(contexto_actual, parametro_1, parametro_2);
        break;

    case EXIT:
        // execute_EXIT(contexto_actual);
        break;

    default:
        log_error(cpu_logger, "Error: Instruccion recibida no recibida.");
        log_error(cpu_logger, "Codigo de instruccion recibida: %d", comando);
        break;
    }
}