#ifndef INICIADOR_MS_H
#define INICIADOR_MS_H

#include "gestor_ms.h"

void imprimir_valores_config();
void ms_cargar_config(char *path);
void ms_liberar_config();
void ms_liberar_log();
void inicializar_logger();

#endif