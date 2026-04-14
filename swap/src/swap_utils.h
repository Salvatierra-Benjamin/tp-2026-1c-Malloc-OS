#ifndef SWAP_UTILS_H
#define SWAP_UTILS_H
#include <stdint.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
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

typedef struct
{
    char *swap_file_path;
    int swap_file_size;
    int swap_block_size;
    char *puerto_km;
    char *ip_km;
    t_log_level log_level;
} swap_config_struct;

typedef struct
{
    int swap_file_size;  // FIFO, RR, CMN
    int swap_block_size; // lista entre FIFO, RR
} parametros_swap;

void swap_cargar_config(char *path, swap_config_struct *swap_config);
void swap_liberar_config(swap_config_struct *swap_config, t_log *swap_logger);
void swap_liberar_log(t_log *swap_logger);

#endif