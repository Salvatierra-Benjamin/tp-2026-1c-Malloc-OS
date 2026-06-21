#ifndef EXECUTE_H
#define EXECUTE_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

typedef struct
{
    void *puntero;
    size_t tamanio;
} t_info_registro;

// AUXILIARES
t_info_registro obtener_info_registro(t_contexto_ejecucion *contexto, char *nombre_registro);
uint32_t leer_registro(t_contexto_ejecucion *contexto, char *registro);
void escribir_registro(t_contexto_ejecucion *contexto, char *registro, uint32_t valor);
// FIN AUXILIARES


// EXECUTE DE INSTRUCCIONES
void execute_NOOP(t_contexto_ejecucion *contexto, char*, char*);
void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str);
void execute_SUM(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_SUB(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_JNZ(t_contexto_ejecucion *contexto, char *registro, char *instruccion_str);

void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char*);
void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos, char*);
void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *tamano_str, char*);

void execute_MUTEX_CREATE(t_contexto_ejecucion *contexto, char *nombre_mutex, char*);;
void execute_MUTEX_LOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char*); 
void execute_MUTEX_UNLOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char*); 

void execute_MEM_ALLOC(t_contexto_ejecucion *contexto, char *tamano_str, char *registro_direccion); 
void execute_MEM_FREE(t_contexto_ejecucion *contexto, char *registro_direccion, char*);

void execute_SLEEP(t_contexto_ejecucion *contexto, char *tiempo_str, char*); 
void execute_STDOUT(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano); 
void execute_STDIN(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano); 

void execute_INIT_PROC(t_contexto_ejecucion *contexto, char *path, char *prioridad_str); 
void execute_EXIT(t_contexto_ejecucion *contexto, char*, char*);
// FIN EXECUTE DE INSTRUCCIONES






#endif