#include "mmu.h"

// Devuelve el proceso a KS con motivo SEG_FAULT y marca el flag para cortar su
// ciclo de instrucción. NO mata la CPU: ésta sigue atendiendo otros procesos.
static void avisar_seg_fault(int pid)
{
    flag_seg_fault = 1;
    int motivo = AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT;
    t_paquete *paquete_ks = crear_paquete(DESALOJO_PROCESO);
    agregar_a_paquete(paquete_ks, &pid, sizeof(int));
    agregar_a_paquete(paquete_ks, &motivo, sizeof(int));
    enviar_paquete(paquete_ks, conexion_ks);
    eliminar_paquete(paquete_ks);
}

// Estructura auxiliar para devolver el resultado de la MMU
uint32_t mmu_traducir(uint32_t dir_logica, uint32_t bytes_a_operar, t_contexto_ejecucion *contexto, int tam_max_segmento)
{
    // Si ya hubo un seg fault en esta instrucción (ej. COPY_MEM traduce 2 veces),
    // no volvemos a traducir ni a avisar.
    if (flag_seg_fault)
        return 0;

    // Al usar enteros en C, la división ya trunca, simulando el floor()
    int num_segmento = dir_logica / tam_max_segmento;
    int desplazamiento_segmento = dir_logica % tam_max_segmento;

    // Buscar el segmento por id_segmento, NO por posición en el array: la tabla se
    // compacta al liberar (eliminar_segmento hace memmove en KM), así que el índice del
    // array deja de coincidir con el nº de segmento tras un MEM_FREE del medio. KM ya
    // traduce por id (traducir_direccion_logica); indexar posicional acá accedía al
    // segmento equivocado (o fuera de rango). Lo destaparon MEMORIA_PRE_1 y PLANI_MEM.
    segmento_t *segmento = NULL;
    for (int i = 0; i < (int)contexto->tabla_segmentos.cantidad_segmentos; i++)
    {
        if (contexto->tabla_segmentos.segmento[i].id_segmento == num_segmento)
        {
            segmento = &contexto->tabla_segmentos.segmento[i];
            break;
        }
    }

    if (segmento == NULL)
    {
        log_error(cpu_logger, "PID: %d - SEG_FAULT: segmento %d inexistente",
                  contexto->PID, num_segmento);
        avisar_seg_fault(contexto->PID);
        return 0;
    }

    // Validar Segmentation Fault (desplazamiento + tamaño excede el segmento)
    if (desplazamiento_segmento + bytes_a_operar > segmento->tam_seg)
    {
        log_error(cpu_logger, "PID: %d - SEG_FAULT: desplazamiento %d + tam %u supera el segmento %d (tam %u)",
                  contexto->PID, desplazamiento_segmento, bytes_a_operar, num_segmento, segmento->tam_seg);
        avisar_seg_fault(contexto->PID);
        return 0;
    }

    // Cálculo de la dirección física
    uint32_t dir_fisica = segmento->dir_base + desplazamiento_segmento;
    return dir_fisica;
}
