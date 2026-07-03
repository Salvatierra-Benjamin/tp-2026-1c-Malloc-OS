#ifndef RECEPTOR_KM_H
#define RECEPTOR_KM_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "conexiones_cpu/handshakes/handshakes.h"
#include "ciclo_instruccion/ciclo_instruccion.h"
#include "contexto_ejecucion/contexto_ejecucion.h"
#include "ciclo_instruccion/fetch/operaciones_instrucciones.h"
void receptor_km();
void *atencion_km();

#endif