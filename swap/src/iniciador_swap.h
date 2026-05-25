#ifndef INICIADOR_SWAP_H
#define INICIADOR_SWAP_H

#include "gestor_swap.h"
// Declaración Funciones:
void swap_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_swap();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void swap_liberar_config();
void swap_liberar_log();
void liberar_todo();

#endif