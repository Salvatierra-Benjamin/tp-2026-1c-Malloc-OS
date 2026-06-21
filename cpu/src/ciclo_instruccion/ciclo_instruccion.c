#include "ciclo_instruccion.h"

void Ejecutar_Ciclo_de_Instruccion(t_contexto_ejecucion *contexto_actual)
{
    bool seguir_ejecutando = true;

    while (seguir_ejecutando)
    {
        t_instruccion nueva_instruccion;
        // Inicializar en NULL por las dudas, buena práctica:
        nueva_instruccion.parametro_1 = NULL;
        nueva_instruccion.parametro_2 = NULL;

        Iniciar_Fetch(contexto_actual, &nueva_instruccion);

        uint32_t pc_anterior = contexto_actual->registros.PC;

        Iniciar_Decode(contexto_actual, nueva_instruccion); // -> execute

        if (contexto_actual->registros.PC == pc_anterior)
        {
            contexto_actual->registros.PC++;
        }

        Destruir_Instruccion(&nueva_instruccion);

        if (check_interrupt(contexto_actual))
        {
            seguir_ejecutando = false;
        }
    }
    log_info(cpu_logger, "Ciclo de instrucción finalizado para PID: %d", contexto_actual->PID);
}
