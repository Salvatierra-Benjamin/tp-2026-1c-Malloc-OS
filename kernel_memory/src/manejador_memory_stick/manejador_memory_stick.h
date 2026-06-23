#ifndef MANEJADOR_MEMORY_STICK_H
#define MANEJADOR_MEMORY_STICK_H

#include <recursos_globales/recursos_globales.h>

void *manejador_memory_stick(void *arg);
void agregar_memory_stick(int tam_ms, char *ip, char *puerto, int socket_memory_stick);

void aviso_aumento_memoria(t_memory_stick *ms);

#endif