#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H
#include "gestor_cpu.h"

// Definición de Tipos
typedef enum
{
    PC,
    AX,
    BX,
    CX,
    DX,
    EAX,
    EBX,
    ECX,
    EDX,
    SI,
    DI
} reg_id;

// Declaración Variables Globales:
extern t_contexto_ejecucion contexto_ejecucion;
extern reg_id reg;
#endif