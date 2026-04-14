#ifndef CPU_CONFIGURACIONES_H
#define CPU_CONFIGURACIONES_H
#include "main.h"
#include "conexiones.h"

bool cpu_cargar_config(char* path);

void cpu_liberar_config(void);

void cpu_config_set_socket_kernel_scheduler(cpu_config *config, int socket);

void cpu_config_set_socket_kernel_memory(cpu_config *config, int socket);

void cpu_config_set_socket_memory_stick(cpu_config *config, int socket);

#endif // CPU_CONFIGURACIONES_H