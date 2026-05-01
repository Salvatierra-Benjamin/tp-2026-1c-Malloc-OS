#ifndef INSTRUCCION_H
#define INSTRUCCION_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

void NOOP();
void *obtener_reg_puntero(reg_id reg, size_t *size);
void SET(reg_id reg, uint32_t valor);
void SUM(reg_id dest, reg_id orig);
void SUB(reg_id orig, reg_id dest);
void JNZ(reg_id registro, uint32_t instruccion);
#endif