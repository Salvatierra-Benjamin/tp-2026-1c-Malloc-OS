#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

typedef enum
{
    // ---------------- Handshakes ----------------
    // Solicitud

    HANDSHAKE_IO = 1000,   // se recibe al IO (io es quien lo manda)
    HANDSHAKE_CPU = 1001,  // CPU envia HS a MS
    HANDSHAKE_SWAP = 1002, // Swap envia HS a KM
    HANDSHAKE_KS = 1003,   // KS se envia a HS a KM
    HANDSHAKE_MS = 1004,   // MS envia HS a KM
    HANDSHAKE_OK = 1005,   // MS,KS,KM LE ENVIA PARA CPU

    //  Respuesta
    KM_OK_CONEXION = 1100, // KM responde a KS, SWAP, MS
    KS_OK_CONEXION = 1101, // KM responde a KS, CPU, IO
    MS_OK_CONEXION = 1102, // MS responde a CPU

    //  DEBUG
    HANDSHAKE_DEBUG_CLIENT = 1400,

    // ---------------- IO -> SCHEDULER ----------------
    IO_ENVIA_IO = 2000,
    STDIN_IO = 2001,  // ks envia stdin
    STDOUT_IO = 2002, // ks envia stdout
    SLEEP_IO = 2003,  // ks envia sleep

    OK_STDIN = 2004,
    OK_STDOUT = 2005,
    OK_SLEEP = 2006,
    ENVIAR_DATOS_IO = 2007,
    // PAQUETE_STDIN = 2100,
    // PAQUETE_STDOUT = 2101,
    // PAQUETE_STDSLEEP = 2102,

    // ---------------- CPU -> SCHEDULER ----------------
    CPU_IDENTIFICACION = 3000,
    ENVIO_MUTEX_CREATE = 3001, // [tamanioNombre, nombreMutex, pid]
    ENVIO_MUTEX_LOCK = 3002,   // [tamanioNombre, nombreMutex, pid]
    ENVIO_MUTEX_UNLOCK = 3003, // [tamanioNombre, nombreMutex, pid]
    AVISO_DE_INTERRUPCION_DESALOJO = 3004,
    AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT = 3005,
    ENVIO_MEM_ALLOC = 3006,
    ENVIO_MEM_FREE = 3007,
    ENVIO_SLEEP = 3008,
    ENVIO_STDOUT = 3009,
    ENVIO_STDIN = 3010,
    ENVIO_INIT_PROC = 3011,
    ENVIO_EXIT = 3012,
    /* //* Envío de palabras reservadas de CPU a KS
    MUTEX_CREATE,     // NOMBRE_MUTEX
    MUTEX_LOCK,       // NOMBRE_MUTEX
    MUTEX_UNLOCK,     // NOMBRE_MUTEX
    SLEEP,            // TIEMPO, PID
    STDOUT,           // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    STDIN,            // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    */

    // ---------------- CPU -> KM ----------------
    SOLICITUD_CONTEXTO_EJECUCION = 3500, // CPU solicita contexto a KM
    ENVIO_CPU_ID = 3501,
    SOLICITUD_INSTRUCCION = 3502, // cpu solicuta instruccion [pid, pc]
    OK_ALOJAR = 3503,             // cpu

    // ---------------- SWAP -> KM ----------------
    BLOCK_SIZE = 4000,
    PETICION_LECTURA_SWAP = 4102,
    RESPUESTA_LECTURA_SWAP = 4103,
    PETICION_ESCRITURA_SWAP = 4104,
    RESPUESTA_ESCRITURA_SWAP = 4105,

    // ---------------- KS -> KM ----------------
    CREACION_PROCESO = 5000,

    // ---------------- MS -> KM ----------------
    MEMORY_STICK_SIZE = 6000,
    ENVIO_TAMANIO_KM = 6001,
    PETICION_LECTURA_MS = 6102,
    RESPUESTA_LECTURA_MS = 6103,
    PETICION_ESCRITURA_MS = 6104,
    RESPUESTA_ESCRITURA_MS = 6105,

    // ---------------- KS -> CPU ----------------
    ALOJAR_PROCESO = 7000, // KS envia pid a CPU
    INTERRUPCION = 7001,
    DESALOJO_PROCESO = 7002,
    // ---------------- KM -> CPU ----------------
    CONTEXTO_EJECUCION = 8000, // KM envia [t_registros, t_tabla_de_segmentos]
    ENVIO_INSTRUCCION = 8001,
    ACTUALIZAR_CONTEXTO_EJECUCION = 8002,
    // ---------------- CODIGOS DE ERROR -----------
    ERROR_ENVIO_INSTRUCCION = 9001,
    ERROR_ENVIO_CONTEXTO = 9002

} op_code;
typedef enum
{
    NOOP,
    SET,              // REGISTRO , VALOR
    MOV_IN,           // REGISTRO, DATOS
    MOV_OUT,          // REGISTRO, DATOS
    SUM,              // REGISTRO_DESTINO, REGISTRO_ORIGEN
    SUB,              // REGISTRO_DESTINO, REGISTRO_ORIGEN
    JNZ,              // REGISTRO, INSTRUCCION
    COPY_MEM,         // REGISTRO, TAMAÑO
    MUTEX_CREATE,     // NOMBRE_MUTEX
    MUTEX_LOCK,       // NOMBRE_MUTEX
    MUTEX_UNLOCK,     // NOMBRE_MUTEX
    MEM_ALLOC,        // ID_SEGMENTO, TAMAÑO, PID
    MEM_FREE,         // ID_SEGMENTO, PID
    SLEEP,            // TIEMPO, PID
    STDOUT,           // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    STDIN,            // REGISTRO_DIRECCION_LOGICA, REGISTRO_TAMAÑO
    INIT_PROC,        // ARCHIVO_INSTRUCCIONES, PRIORIDAD
    EXIT,             // FIN DE PROCESO
    ERROR_INSTRUCCION // HUBO UN ERROR EN LA INSTRUCCION ENVIADA DE KM A CPU
} t_palabra_reservada;
typedef enum
{
    OK,
    NO,
    ERROR
} t_respuesta;

typedef struct
{
    int size;
    int offset;
    void *stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer *buffer;
} t_paquete;

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
    int id_segmento;
    uint32_t dir_base;
    uint32_t tam_seg;
} segmento_t;

typedef struct
{
    segmento_t *segmento;
    uint32_t cantidad_segmentos;
} tabla_segmentos_t;

typedef struct
{
    int PID;
    t_registros registros;
    tabla_segmentos_t tabla_segmentos;

} t_contexto_ejecucion;

typedef struct
{
    t_palabra_reservada comando;
    int PID;
    char *parametro_1;
    char *parametro_2;
} t_instruccion;

typedef struct
{
    int tamanio;
    int id;
} t_memory_stick; // lista para el km

int enviar_operacion(int socket_destino, op_code codigo_operacion);
int recibir_operacion(int);
t_paquete *crear_paquete(op_code);
void agregar_a_paquete(t_paquete *, void *, int);
int enviar_paquete(t_paquete *, int);
void eliminar_paquete(t_paquete *);
t_paquete *recibir_paquete(int);
void buffer_read(void *, t_buffer *, int);
op_code obtener_codigo_instruccion(char *);
#endif
