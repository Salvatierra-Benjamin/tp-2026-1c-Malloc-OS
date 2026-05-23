#ifndef FETCH_H
#define FETCH_H

#include "gestor_debug_client.h"
#include "iniciador_debug_client.h"

void Iniciar_Fetch(t_contexto_ejecucion *contexto_actual, t_instruccion *nueva_instruccion);
void Solicitar_Instruccion(int PID, int PC);
void Recibir_Instruccion1(t_instruccion *nueva_intruccion);
void Recibir_Instruccion2(t_instruccion *nueva_intruccion);
void Loggear_Instruccion(t_instruccion *instruccion);
void Destruir_Instruccion(t_instruccion *instruccion);

#endif