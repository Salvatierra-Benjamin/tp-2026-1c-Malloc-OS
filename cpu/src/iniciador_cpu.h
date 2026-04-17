#ifndef INICIADOR_CPU_H
#define INICIADOR_CPU_H

#include "gestor_cpu.h"

void imprimir_valores_config_cpu();
void cpu_cargar_config(char *path);
void cpu_liberar_config();
void cpu_liberar_log();
void inicializar_logger();

#endif