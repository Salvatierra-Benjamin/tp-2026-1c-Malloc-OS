#ifndef CONEXIONES_DEBUG_CLIENT_H
#define CONEXIONES_DEBUG_CLIENT_H

#include "gestor_debug_client.h"
#include "iniciador_debug_client.h"
#include "receptor_debug_server.h"

void creacion_conexiones_debug_client();
bool realizar_handshake(int socket_servidor, int id_debug_client, char *nombre_servidor);

#endif