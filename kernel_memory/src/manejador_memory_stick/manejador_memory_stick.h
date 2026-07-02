#ifndef MANEJADOR_MEMORY_STICK_H
#define MANEJADOR_MEMORY_STICK_H

#include <recursos_globales/recursos_globales.h>

void *manejador_memory_stick(void *arg);
void agregar_memory_stick(int tam_ms, char *ip, char *puerto, int socket_memory_stick);

void aviso_aumento_memoria(t_memory_stick *ms);

void abrir_conexion_datos(t_memory_stick *ms);

// Primitivos read/write sobre el espacio de direcciones global (bloqueantes).
// Rutean por lista_datos_ms y parten el pedido si cruza el límite entre sticks.
// El llamador garantiza que [direccion, direccion+tamanio) cae dentro de un segmento válido.
bool leer_rango_global(int pid, uint32_t direccion, uint32_t tamanio, void *destino);
bool escribir_rango_global(int pid, uint32_t direccion, uint32_t tamanio, void *origen);

#endif