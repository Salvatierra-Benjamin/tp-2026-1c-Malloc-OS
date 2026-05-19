#ifndef INICIADOR_CPU_H
#define INICIADOR_CPU_H

#include "gestor_cpu.h"
#include "contexto_ejecucion/contexto_ejecucion.h"
// Declaración Funciones:
void cpu_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_cpu();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void cpu_liberar_config();
void cpu_liberar_log();
void liberar_todo();

#endif