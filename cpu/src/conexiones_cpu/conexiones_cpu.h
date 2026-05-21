#ifndef CONEXIONES_CPU_H
#define CONEXIONES_CPU_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"
#include "receptor_ks.h"

void creacion_conexiones_cpu();
bool realizar_handshake(int socket_servidor, int id_cpu, char *nombre_servidor);

#endif