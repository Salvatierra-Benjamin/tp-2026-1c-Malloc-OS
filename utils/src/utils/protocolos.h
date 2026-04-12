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

typedef enum {
    // ---------------- Handshakes ----------------
    // Solicitud
    HANDSHAKE_IO       = 1000, // se recibe al IO (io es quien lo manda)
    HANDSHAKE_CPU      = 1001,
    HANDSHAKE_SWAP     = 1003, // Swap envia HS a km
    HANDSHAKE_KS       = 1004, // KS se conecta a KM

    // Respuesta
    KM_OK_CONEXION     = 1100, // KM responde a KS
    KS_OK_CONEXION     = 1101, // KS responde a KM
    MS_OK_CONEXION     = 1102, // KS responde a KM

    // ---------------- IO -> SCHEDULER ----------------
    IO_ENVIA_IO        = 2000,

    // ---------------- CPU -> SCHEDULER ----------------
    CPU_IDENTIFICACION = 3000,

    // ---------------- SWAP -> KM ----------------
    BLOCK_SIZE = 4000
} op_code;


typedef enum {
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
t_paquete *crear_paquete(op_code );
void agregar_a_paquete(t_paquete *, void *, int );
int enviar_paquete(t_paquete *, int );
void eliminar_paquete(t_paquete *);
t_paquete* recibir_paquete(int );
void buffer_read(void* , t_buffer* , int ) ;
op_code obtener_codigo_instruccion(char*);
#endif
