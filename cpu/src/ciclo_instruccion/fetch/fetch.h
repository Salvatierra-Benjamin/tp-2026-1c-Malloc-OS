#ifndef FETCH_H
#define FETCH_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

t_instruccion Iniciar_Fetch(t_contexto_ejecucion *contexto_actual);
void Solicitar_Instruccion(int PID, int PC);
t_instruccion Recibir_Instruccion();

#endif