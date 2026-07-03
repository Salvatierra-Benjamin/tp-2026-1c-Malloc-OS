#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H
#include "gestor_cpu.h"
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

void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer);
void Loggear_Contexto(t_contexto_ejecucion *contexto_actual);
void Destruir_Contexto(t_contexto_ejecucion *contexto);
#endif