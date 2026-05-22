#ifndef EXECUTE2_H
#define EXECUTE2_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

typedef struct
{
    void *puntero;
    size_t tamanio;
} t_info_registro;

// void NOOP();
// void *obtener_reg_puntero(reg_id reg, size_t *size);
// void SET(reg_id reg, uint32_t valor);
// void SUM(reg_id dest, reg_id orig);
// void SUB(reg_id orig, reg_id dest);
// void JNZ(reg_id registro, uint32_t instruccion);


t_info_registro obtener_info_registro(t_contexto_ejecucion *contexto, char *nombre_registro);
uint32_t leer_registro(t_contexto_ejecucion *contexto, char *registro);
void escribir_registro(t_contexto_ejecucion *contexto, char *registro, uint32_t valor);
void execute_NOOP(t_contexto_ejecucion *contexto);
void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str);
void execute_SUM(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_SUB(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_JNZ(t_contexto_ejecucion *contexto, char *registro, char *instruccion_str);








#endif