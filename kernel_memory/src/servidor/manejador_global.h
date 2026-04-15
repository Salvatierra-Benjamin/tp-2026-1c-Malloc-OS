#ifndef MANEJADOR_GLOBAL
#define MANEJADOR_GLOBAL

#include <recursos_globales/recursos_globales.h>
#include <manejador_ks/manejador_ks.h>
#include <manejador_cpu/manejador_cpu.h>
#include <manejador_memory_stick/manejador_memory_stick.h>
#include <manejador_swap/manejador_swap.h>

void entrada_dispositivos(int socket_servidor);
void *seleccion_dispositivo(void *arg);
void delegar_a(int socket_cliente, void*(*manejador)(void*), const char* nombre_modulo);

#endif