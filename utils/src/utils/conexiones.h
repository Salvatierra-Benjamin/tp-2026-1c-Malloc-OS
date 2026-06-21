#ifndef CONEXIONES_H_
#define CONEXIONES_H_

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
#include <stdbool.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <pthread.h>
#include <netdb.h>

int iniciar_servidor(char *);
int esperar_cliente(int);
int crear_conexion(char *ip, char *puerto);
#endif
