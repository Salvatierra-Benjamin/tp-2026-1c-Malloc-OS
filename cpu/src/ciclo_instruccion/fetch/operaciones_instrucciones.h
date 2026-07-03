#ifndef OPERACIONES_INSTRUCCIONES_H
#define OPERACIONES_INSTRUCCIONES_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

void Solicitar_Instruccion(int PID, int PC);
void Recibir_Instruccion(t_instruccion *nueva_intruccion);
void Loggear_Instruccion(t_instruccion *instruccion);
void Destruir_Instruccion(t_instruccion *instruccion);

#endif