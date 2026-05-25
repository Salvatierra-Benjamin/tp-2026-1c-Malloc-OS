#ifndef MAIN_DEBUG_CLIENT_H
#define MAIN_DEBUG_CLIENT_H

#include "gestor_debug_client.h"
#include "iniciador_debug_client.h"
#include "conexiones_debug_client/conexiones_debug_client.h"
#include "ciclo_instruccion/ciclo_instruccion.h"
void Recibir_Contexto(t_contexto_ejecucion *contexto, t_buffer *buffer);
void Loggear_Contexto(t_contexto_ejecucion *contexto_actual);
void Destruir_Contexto(t_contexto_ejecucion *contexto);
#endif