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

int conectar_a_kernel_memory(void)
{
     char *ipMemoria = cpu_config_get_ip_kernel_memory(cpuConfig);
     char *puertoMemoria = cpu_config_get_puerto_kernel_memory(cpuConfig);


    return socketKernelMemory;
}
int conectar_a_kernel_scheduler(void);
int conectar_a_memory_stick(void);

