
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
bool cpu_cargar_config(char* path);

void cpu_liberar_config(void);

uint32_t cpu_obtener_ip_kernel_memory(cpu_config* config);
uint32_t cpu_obtener_ip_kernel_scheduler(cpu_config* config);
uint32_t cpu_obtener_ip_memory_stick(cpu_config* config);

int cpu_obtener_puerto_kernel_memory(cpu_config* config);
int cpu_obtener_puerto_kernel_scheduler(cpu_config* config);
int cpu_obtener_puerto_memory_stick(cpu_config* config);
