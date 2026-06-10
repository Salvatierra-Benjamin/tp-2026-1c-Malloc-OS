
#include "check_interrupt.h"

bool check_interrupt(t_contexto_ejecucion *contexto_actual)
{
    // sem_trywait devuelve 0 (hay interrupción) → ejecuta envío → retorna true → rompe while
    // sem_trywait devuelve -1 (no hay) → retorna false → sigue el while.
    if (sem_trywait(&hay_interrupcion) != 0) //
    {
        return false;
    }

    log_info(cpu_logger, "[CPU] Interrupción recibida para PID %d", contexto_actual->PID);

    // 1) Enviar contexto actualizado a Kernel Memory
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

    //  2) Notificar al Kernel Scheduler que se devuelve el PID por interrupción
    //  Motivo: como receptor_ks sólo notifica la existencia de la interrupción,
    //  usamos un motivo genérico (1 = PREEMPTIVO). Ajustar si se cambia protocolo.
    sem_post(&notificar_ks);

    // 3)El CPU debe dejar de ejecutar este proceso; la lógica de control superior
    // (Ejecutar_Ciclo_de_Instruccion) detectará el cambio de estado tras la vuelta.
    return true; // Informamos al while que DEBE detenerse
}
