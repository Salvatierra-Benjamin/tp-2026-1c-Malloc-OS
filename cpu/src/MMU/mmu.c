#include "mmu.h"

int MMU_TRADUCCION_FISICA(int dir_logica, uint32_t tam, int *dir_fisica, uint32_t bytes_validos)
{

    tam_max_segmentos = Get_tam_max_segmentos();

    uint32_t num_segmento = floor(dir_logica / tam_max_segmentos);
    uint32_t desplazamiento = dir_logica % tam_max_segmentos;

    // segmento invalido
    if (num_segmento >= contexto_ejecucion.tabla_s.cantidad_segmentos * tam)
    {
        log_error(cpu_logger, "Seg. Invalido");
        return -1;
    }
    segmento_t seg = contexto_ejecucion.tabla_s.tabla_segmentos[num_segmento];
    // segmentation fault
    if (desplazamiento >= seg.tam_seg)
    {
        log_error(cpu_logger, "Seg. fault en funcion MMU_TRADUCCION_FISICA()");
        return -2;
    }

    uint32_t dispo = (seg.tam_seg - desplazamiento);
    bytes_validos = (tam < dispo) ? tam : dispo;
    dir_fisica = seg.dir_base + desplazamiento;
    return 0;
}

// liberar_todo(); solo si dice exit(EXIT_FAILURE)

// log_error(cpu_logger, "cod op %d", cod_op);
// log_error(cpu_logger, "KM desconectado o error");
// liberar_todo();
// exit(EXIT_FAILURE);