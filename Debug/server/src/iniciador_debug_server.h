#ifndef INICIADOR_DEBUG_SERVER_H
#define INICIADOR_DEBUG_SERVER_H

#include "gestor_debug_server.h"
// Declaración Funciones:
void debug_server_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_debug_server();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void debug_server_liberar_config();
void debug_server_liberar_log();
void liberar_todo();

#endif