#ifndef RECURSOS_GLOBALES
#define RECURSOS_GLOBALES

#include <utils/conexiones.h>
#include <utils/protocolos.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>

typedef struct
{
    char *puerto_escucha;
    t_log_level log_level;
    int segment_max_size;
    char *allocation_strategy;
    int instruction_delay;
    int compaction_delay;
    char *scripts_basepath;
} km_config;

typedef enum
{
    NOOP,
    SET, // REGISTRO , VALOR
    MOV_IN, // REGISTRO, DATOS
    MOV_OUT, // REGISTRO, DATOS
    SUM, // REGISTRO_DESTINO, REGISTRO_ORIGEN
    SUB, // REGISTRO_DESTINO, REGISTRO_ORIGEN
    JNZ, // REGISTRO, INSTRUCCION
    COPY_MEM, // REGISTRO, TAMAÑO
    MUTEX_CREATE, // NOMBRE_MUTEX
    MUTEX_LOCK, // NOMBRE_MUTEX
    MUTEX_UNLOCK, // NOMBRE_MUTEX
    MEM_ALLOC, // ID_SEGMENTO, TAMAÑO
    MEM_FREE, // ID_SEGMENTO
    SLEEP, // TIEMPO
    STDOUT, // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    STDIN, // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    INIT_PROC, // ARCHIVO_INSTRUCCIONES, PRIORIDAD
    EXIT
} t_palabra_reservada;

typedef struct
{
    t_palabra_reservada comando;
    char *parametro_1;
    char *parametro_2;
} t_instruccion;

typedef struct
{
    uint32_t pc;
    uint8_t ax, bx, cx, dx;
    uint32_t eax, ebx, ecx, edx;
    uint32_t si, di;
} t_registros_cpu;

typedef struct
{
    t_registros_cpu registros;
    t_list *tabla_segmentos;
} t_contexto_ejecucion;

typedef struct
{
    t_list *instrucciones; // t_instruccion []
    t_contexto_ejecucion *contexto;
} t_procesos_memoria;

extern t_dictionary *procesos_memoria;
extern pthread_mutex_t mutex_procesos_memoria;

extern t_log *logger;
extern t_config *config;
extern km_config *parametros;

void iniciar_recursos();
void liberar_recursos();

#endif