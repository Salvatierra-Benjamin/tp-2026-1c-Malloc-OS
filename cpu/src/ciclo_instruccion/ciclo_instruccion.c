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

        // Se resetea antes del execute: lo setea escribir_registro/execute_JNZ si la
        // instrucción escribe el PC. Reemplaza la comparación por valor (que fallaba
        // cuando un salto apuntaba al mismo PC que ya tenía).
        flag_pc_modificado = 0;

        Iniciar_Decode(contexto_actual, nueva_instruccion); // -> execute

        // SEG_FAULT: el proceso ya fue devuelto a KS (motivo SEG_FAULT). Cortamos el
        // ciclo sin PC++, sin actualizar contexto y sin chequear interrupción.
        // La CPU vuelve a esperar otro proceso (NO se mata).
        if (flag_seg_fault == 1)
        {
            Destruir_Instruccion(&nueva_instruccion);
            seguir_ejecutando = false;
            break;
        }

        if (!flag_pc_modificado)
        {
            contexto_actual->registros.PC++;
        } // la instrucción no tocó el PC: avanzamos a la siguiente

        // Syscall Desalojo CPU, Libero y espero otro proceso.
        if (flag_syscall == 1)
        {
            t_paquete *paquete_km = crear_paquete(ACTUALIZAR_CONTEXTO_EJECUCION);
            agregar_a_paquete(paquete_km, &(contexto_actual->PID), sizeof(int));
            agregar_a_paquete(paquete_km, &(contexto_actual->registros), sizeof(t_registros));
            agregar_a_paquete(paquete_km, &(contexto_actual->tabla_segmentos.cantidad_segmentos), sizeof(uint32_t));
            if (contexto_actual->tabla_segmentos.cantidad_segmentos > 0 && contexto_actual->tabla_segmentos.segmento != NULL)
            {
                size_t tam = sizeof(segmento_t) * contexto_actual->tabla_segmentos.cantidad_segmentos;
                agregar_a_paquete(paquete_km, contexto_actual->tabla_segmentos.segmento, tam);
            }
            enviar_paquete(paquete_km, conexion_km);
            eliminar_paquete(paquete_km);
            seguir_ejecutando = false;
        }

        Destruir_Instruccion(&nueva_instruccion);

        // Solo chequeamos interrupción si NO fue una syscall: si lo fue, el proceso
        // ya se devolvió al KS por esa vía y ya se mandó el contexto a KM. Chequear
        // acá mandaría otro ACTUALIZAR_CONTEXTO + DESALOJO_PROCESO espurio.
        if (flag_syscall != 1 && check_interrupt(contexto_actual))
        {
            seguir_ejecutando = false;
        }
    }
    log_info(cpu_logger, "Ciclo de instrucción finalizado para PID: %d", contexto_actual->PID);
}
