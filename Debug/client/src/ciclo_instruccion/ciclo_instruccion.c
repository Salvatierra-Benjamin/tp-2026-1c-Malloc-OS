#include "ciclo_instruccion.h"
void Ejecutar_Ciclo_de_Instruccion(t_contexto_ejecucion *contexto_actual)
{
    t_instruccion nueva_intruccion;
    Iniciar_Fetch(contexto_actual, &nueva_intruccion);
}