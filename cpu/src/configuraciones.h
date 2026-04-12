#ifndef CPU_CONFIGURACIONES_H
#define CPU_CONFIGURACIONES_H
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include<main.h>
#include<conexiones.h>

bool cpu_cargar_config(char* path);

void cpu_liberar_config(void);

void cpu_config_set_socket_kernel_scheduler(cpu_config *config, int socket);

void cpu_config_set_socket_kernel_memory(cpu_config *config, int socket);

void cpu_config_set_socket_memory_stick(cpu_config *config, int socket);
