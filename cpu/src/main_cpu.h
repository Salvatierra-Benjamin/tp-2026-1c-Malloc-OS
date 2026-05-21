#ifndef MAIN_CPU_H
#define MAIN_CPU_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "conexiones_cpu/conexiones_cpu.h"
#include "ciclo_instruccion/ciclo_instruccion.h"
void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer);
void Loggear_Contexto(t_contexto_ejecucion *contexto_actual);
void Destruir_Contexto(t_contexto_ejecucion *contexto);
#endif