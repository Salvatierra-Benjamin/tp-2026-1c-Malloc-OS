#ifndef MANEJADOR_SWAP_H
#define MANEJADOR_SWAP_H

#include <recursos_globales/recursos_globales.h>

void *manejador_swap(void *arg);
void agregar_swap(int tam_bloque, int tam_swap);

// Allocator de bloques de swap (bitmap bloques_swap_ocupados).
// reservar: devuelve lista de int* con los números de bloque reservados, o NULL si no
// alcanzan (en ese caso no reserva ninguno). Con cantidad 0 devuelve lista vacía.
// liberar: marca los bloques libres y destruye la lista.
t_list *reservar_bloques_swap(int cantidad);
void liberar_bloques_swap(t_list *bloques);

// Petición+respuesta sincrónicas sobre socket_swap (par completo bajo mutex_socket_swap).
// 'datos'/'destino' deben tener tam_bloque_swap bytes. Devuelven false si swap no está
// conectado, se cae a mitad de la operación o responde exito == 0.
// Contrato (errores-swap-ms.md):
//   PETICION_ESCRITURA_SWAP [bloque][datos] -> RESPUESTA_ESCRITURA_SWAP [bloque][exito]
//   PETICION_LECTURA_SWAP   [bloque]        -> RESPUESTA_LECTURA_SWAP   [bloque][exito][datos si exito]
bool escribir_bloque_swap(int bloque, void *datos);
bool leer_bloque_swap(int bloque, void *destino);

#endif
