#include "mmu.h"
// IMPLEMENTACION MMU con parametros de entrada por referencia (para evitar confusiones con el paso por valor de bytes_validos y dir_fisica)
int MMU_TRADUCCION_FISICA(int dir_logica, uint32_t tam, int *dir_fisica, uint32_t *bytes_validos)
{
    uint32_t tam_max_segmentos = Get_tam_max_segmentos();
    uint32_t num_segmento = dir_logica / tam_max_segmentos;
    uint32_t desplazamiento = dir_logica % tam_max_segmentos;

    if (num_segmento >= contexto_ejecucion.tabla_segmentos.cantidad_segmentos)
    {
        log_error(cpu_logger, "Seg. Invalido");
        liberar_todo();

        exit(EXIT_FAILURE);
    }

    segmento_t seg = contexto_ejecucion.tabla_segmentos.segmento[num_segmento];
    if (desplazamiento >= seg.tam_seg)
    {
        log_error(cpu_logger, "Seg. fault en funcion MMU_TRADUCCION_FISICA()");

        int motivo = AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT;
        t_paquete *paquete_ks = crear_paquete(DESALOJO_PROCESO);
        int pid = contexto_actual->PID;
        agregar_a_paquete(paquete_ks, &pid, sizeof(int));
        agregar_a_paquete(paquete_ks, &motivo, sizeof(int));
        enviar_paquete(paquete_ks, conexion_ks);
        eliminar_paquete(paquete_ks);
        liberar_todo();

        exit(EXIT_FAILURE);
    }

    uint32_t dispo = seg.tam_seg - desplazamiento;
    *bytes_validos = (tam < dispo) ? tam : dispo;
    *dir_fisica = seg.dir_base + desplazamiento;
    return 0;
}

// Estructura auxiliar para devolver el resultado de la MMU
t_mmu_result mmu_traducir(uint32_t dir_logica, uint32_t bytes_a_operar, t_contexto_ejecucion *contexto, int tam_max_segmento)
{
    t_mmu_result resultado = {0, false};

    // Al usar enteros en C, la división ya trunca, simulando el floor()
    int num_segmento = dir_logica / tam_max_segmento;
    int desplazamiento_segmento = dir_logica % tam_max_segmento;

    // Validar si el segmento existe en la tabla
    if (num_segmento >= contexto->tabla_segmentos.cantidad_segmentos)
    {
        resultado.seg_fault = true; // TODO COPIAR SEGMENTATION FAULT MENSAJE
        return resultado;
    }

    segmento_t segmento = contexto->tabla_segmentos.segmento[num_segmento];

    // Validar Segmentation Fault
    if (desplazamiento_segmento + bytes_a_operar > segmento.tam_seg)
    {
        resultado.seg_fault = true; // TODO COPIAR SEGMENTATION FAULT MENSAJE
        return resultado;
    }

    // Cálculo de la dirección física
    resultado.dir_fisica = segmento.dir_base + desplazamiento_segmento;
    return resultado;
}