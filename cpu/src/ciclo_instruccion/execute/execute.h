#ifndef EXECUTE_H
#define EXECUTE_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "MMU/mmu.h"
typedef struct
{
    void *puntero;
    size_t tamanio;
} t_info_registro;
typedef enum
{
    LEER,
    ESCRIBIR
} t_operacion_memoria;

//===============================================
//    FUNCIONES AUXILIARES
//===============================================

t_info_registro obtener_info_registro(t_contexto_ejecucion *contexto, char *nombre_registro);
uint32_t leer_registro(t_contexto_ejecucion *contexto, char *registro);
void escribir_registro(t_contexto_ejecucion *contexto, char *registro, uint32_t valor);
int pasaje_de_char_a_int(char *palabra);
t_memory_stick *encontrar_stick_por_direccion(t_list *sticks, uint32_t dir_fisica);
void operar_en_memoria(int pid, uint32_t dir_fisica, void *buffer, uint32_t tamano, t_operacion_memoria op, t_list *memory_sticks);
void setear_valor_registro(t_contexto_ejecucion *contexto, char *registro_datos, void *buffer);
int obtener_tamano_registro(char *registro_datos);
uint32_t obtener_valor_registro_como_entero(t_contexto_ejecucion *contexto, char *nombre_registro_tamanio);

// =========================================================
// EXECUTE DE INSTRUCCIONES
// =========================================================

// =========================================================
// INSTRUCCIONES DE CPU PURA (Matematica y Control)
// =========================================================
void execute_NOOP(t_contexto_ejecucion *contexto, char *param1, char *param2);
void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str);
void execute_SUM(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_SUB(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen);
void execute_JNZ(t_contexto_ejecucion *contexto, char *registro, char *instruccion_str);

// =========================================================
// INSTRUCCIONES DE MEMORIA (MMU)
// =========================================================
void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char *param2);
void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos, char *param2);
void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *nombre_registro_tamano, char *param2);

// =========================================================
// SYSCALLS
// =========================================================
void execute_MUTEX_CREATE(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2);
void execute_MUTEX_LOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2);
void execute_MUTEX_UNLOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2);

void execute_MEM_ALLOC(t_contexto_ejecucion *contexto, char *tamano_str, char *registro_direccion);
void execute_MEM_FREE(t_contexto_ejecucion *contexto, char *registro_direccion, char *param2);

void execute_SLEEP(t_contexto_ejecucion *contexto, char *tiempo_str, char *param2);
void execute_STDOUT(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano);
void execute_STDIN(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano);

void execute_INIT_PROC(t_contexto_ejecucion *contexto, char *path, char *prioridad_str);
void execute_EXIT(t_contexto_ejecucion *contexto, char *param1, char *param2);
// FIN EXECUTE DE INSTRUCCIONES

#endif