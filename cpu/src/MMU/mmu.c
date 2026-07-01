#include "mmu.h"

// Estructura auxiliar para devolver el resultado de la MMU
uint32_t mmu_traducir(uint32_t dir_logica, uint32_t bytes_a_operar, t_contexto_ejecucion *contexto, int tam_max_segmento)
{

    // Al usar enteros en C, la división ya trunca, simulando el floor()
    int num_segmento = dir_logica / tam_max_segmento;
    int desplazamiento_segmento = dir_logica % tam_max_segmento;

    // Validar si el segmento existe en la tabla
    if (num_segmento >= contexto->tabla_segmentos.cantidad_segmentos)
    {
        int motivo = AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT;
        t_paquete *paquete_ks = crear_paquete(DESALOJO_PROCESO);
        int pid = contexto->PID;
        agregar_a_paquete(paquete_ks, &pid, sizeof(int));
        agregar_a_paquete(paquete_ks, &motivo, sizeof(int));
        enviar_paquete(paquete_ks, conexion_ks);
        eliminar_paquete(paquete_ks);
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    segmento_t segmento = contexto->tabla_segmentos.segmento[num_segmento];

    // Validar Segmentation Fault
    if (desplazamiento_segmento + bytes_a_operar > segmento.tam_seg)
    {
        int motivo = AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT;
        t_paquete *paquete_ks = crear_paquete(DESALOJO_PROCESO);
        int pid = contexto->PID;
        agregar_a_paquete(paquete_ks, &pid, sizeof(int));
        agregar_a_paquete(paquete_ks, &motivo, sizeof(int));
        enviar_paquete(paquete_ks, conexion_ks);
        eliminar_paquete(paquete_ks);
        liberar_todo();
        exit(EXIT_FAILURE);
    }

    // Cálculo de la dirección física
    uint32_t dir_fisica = segmento.dir_base + desplazamiento_segmento;
    return dir_fisica;
}