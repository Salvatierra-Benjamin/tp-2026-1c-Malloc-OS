#ifndef INICIADOR_DEBUG_CLIENT_H
#define INICIADOR_DEBUG_CLIENT_H

#include "gestor_debug_client.h"

// Declaración Funciones:
void debug_client_cargar_config(char *path);
void inicializar_logger();
void imprimir_valores_config_debug_client();
void inicializar_semaforos_coordinadores();
void cerrar_conexiones();
void liberar_semaforos();
void debug_client_liberar_config();
void debug_client_liberar_log();
void liberar_todo();

#endif