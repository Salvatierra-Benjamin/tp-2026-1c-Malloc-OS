#ifndef INICIADOR_MS_H
#define INICIADOR_MS_H

#include "gestor_ms.h"
// Declaración Funciones:
void ms_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_ms();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void ms_liberar_config();
void ms_liberar_log();
void liberar_todo();

#endif