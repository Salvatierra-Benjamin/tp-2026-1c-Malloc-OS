#ifndef CHECK_INTERRUPT_H
#define CHECK_INTERRUPT_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "contexto_ejecucion/contexto_ejecucion.h"

bool check_interrupt(t_contexto_ejecucion *contexto_actual);

#endif