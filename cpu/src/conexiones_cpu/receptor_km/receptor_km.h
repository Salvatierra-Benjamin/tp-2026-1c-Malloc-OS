#ifndef RECEPTOR_KM_H
#define RECEPTOR_KM_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "conexiones_cpu/handshakes/handshakes.h"
#include "ciclo_instruccion/ciclo_instruccion.h"
void receptor_km();
void *atencion_km();
void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer);
void Loggear_Contexto(t_contexto_ejecucion *contexto_actual);
void Destruir_Contexto(t_contexto_ejecucion *contexto);
#endif