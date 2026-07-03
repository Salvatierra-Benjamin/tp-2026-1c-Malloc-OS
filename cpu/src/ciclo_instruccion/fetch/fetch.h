#ifndef FETCH_H
#define FETCH_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "operaciones_instrucciones.h"
void Iniciar_Fetch(t_contexto_ejecucion *contexto_actual, t_instruccion *nueva_instruccion);

#endif