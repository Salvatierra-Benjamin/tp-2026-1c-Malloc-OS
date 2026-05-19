#ifndef INICIADOR_IO_H
#define INICIADOR_IO_H

#include "gestor_io.h"
// Declaración Funciones:
char *normalizar(char *tipo_io);
void imprimir_tipo_de_io(char *argumento_mayusculas);
void io_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_io();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void io_liberar_config();
void io_liberar_log();
void liberar_todo();

#endif