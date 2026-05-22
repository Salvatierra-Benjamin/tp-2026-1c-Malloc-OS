#ifndef DECODE_H
#define DECODE_H

#include "gestor_cpu.h"
#include "../../contexto_ejecucion/contexto_ejecucion.h"

void Decode_Instruccion();
typedef void (*t_executor)(t_contexto_ejecucion*, char*, char*);

// Tabla de saltos indexada por el comando
t_executor tabla_comandos[] = {
    [NOOP]         = execute_NOOP,
    [SET]          = execute_SET,
    [MOV_IN]       = execute_MOV_IN,
    [MOV_OUT]      = execute_MOV_OUT,
    [SUM]          = execute_SUM,
    [SUB]          = execute_SUB,
    [JNZ]          = execute_JNZ,
    [COPY_MEM]     = execute_COPY_MEM,
    [MUTEX_CREATE] = execute_MUTEX_CREATE,
    [MUTEX_LOCK]   = execute_MUTEX_LOCK,
    [MUTEX_UNLOCK] = execute_MUTEX_UNLOCK,
    [MEM_ALLOC]    = execute_MEM_ALLOC,
    [MEM_FREE]     = execute_MEM_FREE,
    [SLEEP]        = execute_SLEEP,
    [STDOUT]       = execute_STDOUT,
    [STDIN]        = execute_STDIN,
    [INIT_PROC]    = execute_INIT_PROC,
    [EXIT]         = execute_EXIT
};
#endif