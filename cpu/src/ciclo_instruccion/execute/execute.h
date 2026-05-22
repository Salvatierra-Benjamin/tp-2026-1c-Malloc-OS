#ifndef EXECUTE_H
#define EXECUTE_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

typedef struct
{
    void *puntero;
    size_t tamanio;
} t_info_registro;

void NOOP();
void *obtener_reg_puntero(reg_id reg, size_t *size);
void SET(reg_id reg, uint32_t valor);
void SUM(reg_id dest, reg_id orig);
void SUB(reg_id orig, reg_id dest);
void JNZ(reg_id registro, uint32_t instruccion);
void execute_NOOP(t_contexto_ejecucion *contexto, char *P1, char *P2);
void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str);
void execute_SUM(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_SUB(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_JNZ(t_contexto_ejecucion *contexto, char *registro, char *instruccion_str);
void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char *P2);
void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos, char *P2);
void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *tamano_str, char *P2);
void execute_MUTEX_CREATE(t_contexto_ejecucion *contexto, char *recurso, char *P2);
void execute_MUTEX_LOCK(t_contexto_ejecucion *contexto, char *recurso, char *P2);
void execute_MUTEX_UNLOCK(t_contexto_ejecucion *contexto, char *recurso, char *P2);
void execute_MEM_ALLOC(t_contexto_ejecucion *contexto, char *tamano_str, char *registro_direccion);
void execute_MEM_FREE(t_contexto_ejecucion *contexto, char *registro_direccion, char *P2);
void execute_SLEEP(t_contexto_ejecucion *contexto, char *tiempo_str, char *P2);
void execute_STDOUT(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano);
void execute_STDIN(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano);
void execute_INIT_PROC(t_contexto_ejecucion *contexto, char *path, char *prioridad_str);
void execute_EXIT(t_contexto_ejecucion *contexto, char *P1, char *P2);
#endif