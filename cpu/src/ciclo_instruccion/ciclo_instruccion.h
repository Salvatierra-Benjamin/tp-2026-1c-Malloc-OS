#ifndef CICLO_INSTRUCCION_H
#define CICLO_INSTRUCCION_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "fetch/fetch.h"
#include "decode/decode.h"
#include "execute/execute.h"
#include "check_interrupt/check_interrupt.h"

void Ejecutar_Ciclo_de_Instruccion(t_contexto_ejecucion *contexto_actual);

#endif