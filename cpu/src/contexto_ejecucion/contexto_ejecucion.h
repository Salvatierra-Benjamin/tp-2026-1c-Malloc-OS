#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H
#include "gestor_cpu.h"

// Definición de Tipos:
typedef struct
{
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;

} t_registros;

typedef struct
{
    int hola_mundo; // TODO BORRADOR
} t_segmentos;

typedef struct
{
    int PDI;
    t_registros registros;
    t_segmentos tabla_segmentos;

} t_contexto_ejecucion;

// Declaración Variables Globales:
extern t_contexto_ejecucion contexto_ejecucion;

#endif