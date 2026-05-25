#ifndef ESPERAR_DEBUG_CLIENT_H
#define ESPERAR_DEBUG_CLIENT_H

#include "gestor_debug_server.h"
#include "iniciador_debug_server.h"

void esperar_debug_client();
void *atencion_debug_client(void *socket_cliente_puntero);

#endif