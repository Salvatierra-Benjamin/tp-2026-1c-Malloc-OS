#ifndef RECEPTOR_KM_H
#define RECEPTOR_KM_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "../ciclo-instruccion/fetch/fetch.h"
// #include "../ciclo-instruccion/fetch/decode.h"
// #include "../ciclo-instruccion/fetch/execute.h"
// #include "../ciclo-instruccion/fetch/check-interrupt.h"

void receptor_km();
void *atencion_km();

#endif