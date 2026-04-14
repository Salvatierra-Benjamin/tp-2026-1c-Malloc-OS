#ifndef CPU_CONEXIONES_H;   
#define CPU_CONEXIONES_H;
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include <../../utils/src/utils/conexiones.h>
#include <../../utils/src/utils/protocolos.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void realizar_handshake(int socket, int cod_op, char* nombre_modulo);

int conectar_a_kernel_memory(void);

int conectar_a_kernel_scheduler(void);

int conectar_a_memory_stick(void);

void conectar_a_nuevo_memory_stick(char* ip, char* puerto);

#endif // CPU_CONEXIONES_H