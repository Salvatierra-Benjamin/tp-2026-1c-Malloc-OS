#ifndef DECODE_H
#define DECODE_H

#include "gestor_cpu.h"
#include "contexto_ejecucion/contexto_ejecucion.h"
#include "ciclo_instruccion/execute/execute.h"

void Iniciar_Decode(t_contexto_ejecucion *contexto_actual, t_instruccion instruccion);
typedef void (*t_executor)(t_contexto_ejecucion *, char *, char *);

#endif