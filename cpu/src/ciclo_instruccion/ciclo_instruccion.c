#include "ciclo_instruccion.h"
void Ejecutar_Ciclo_de_Instruccion(t_contexto_ejecucion *contexto_actual)
{
    t_instruccion nueva_instruccion;
    // Inicializar en NULL por las dudas, buena práctica:
    nueva_instruccion.parametro_1 = NULL;
    nueva_instruccion.parametro_2 = NULL;

    Iniciar_Fetch(contexto_actual, &nueva_instruccion);
    Iniciar_Decode(contexto_actual, nueva_instruccion);
    Iniciar_Execute(contexto_actual);
    Destruir_Instruccion(nueva_instruccion);
}