#ifndef MANEJADOR_GLOBAL
#define MANEJADOR_GLOBAL

#include <recursos_globales/recursos_globales.h>
#include <manejador_ks/manejador_ks.h>

void entrada_dispositivos(int socket_servidor);
void *seleccion_dispositivo(void *arg);

#endif