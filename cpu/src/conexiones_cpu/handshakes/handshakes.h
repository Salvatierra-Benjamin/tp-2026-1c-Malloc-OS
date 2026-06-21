#ifndef HANDSHAKES_H
#define HANDSHAKES_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

bool realizar_handshake(int socket_servidor, int id_cpu, char *nombre_servidor);
bool realizar_handshake_ks(int socket_servidor, int id_cpu, char *nombre_servidor);

#endif