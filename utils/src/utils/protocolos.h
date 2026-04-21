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

    // Respuesta
    KM_OK_CONEXION = 1100, // KM responde a KS, SWAP, MS
    KS_OK_CONEXION = 1101, // KM responde a KS, CPU, IO
    MS_OK_CONEXION = 1102, // MS responde a CPU

    // ---------------- IO -> SCHEDULER ----------------
    IO_ENVIA_IO = 2000,
    STDIN_IO = 2001,  // ks envia stdin
    STDOUT_IO = 2002, // ks envia stdout
    SLEEP_IO = 2003,  // ks envia sleep

    OK_STDIN = 2004,
    OK_STDOUT = 2005,
    OK_SLEEP = 2006,

    // PAQUETE_STDIN = 2100,
    // PAQUETE_STDOUT = 2101,
    // PAQUETE_STDSLEEP = 2102,

    // ---------------- CPU -> SCHEDULER ----------------
    CPU_IDENTIFICACION = 3000,
    SOLICITUD_CONTEXTO_EJECUCION = 3001, // CPU solicita contexto a KM
    ENVIO_CPU_ID = 3002,

    // ---------------- SWAP -> KM ----------------
    BLOCK_SIZE = 4000,

    // ---------------- KS -> KM ----------------
    CREACION_PROCESO = 5000,

    // ---------------- MS -> KM ----------------
    MEMORY_STICK_SIZE = 6000,

} op_code;

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
