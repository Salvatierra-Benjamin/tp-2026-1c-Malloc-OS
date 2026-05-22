#ifndef DECODE_H
#define DECODE_H

#include "gestor_cpu.h"
#include "../../contexto_ejecucion/contexto_ejecucion.h"
#include "../execute/execute.h"

// void Decode_Instruccion();
void Iniciar_Decode(t_contexto_ejecucion *contexto_actual, t_instruccion instruccion);


#endif