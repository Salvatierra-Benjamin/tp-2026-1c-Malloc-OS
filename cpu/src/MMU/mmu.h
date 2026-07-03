#ifndef MMU_H
#define MMU_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

uint32_t mmu_traducir(uint32_t dir_logica, uint32_t bytes_a_operar, t_contexto_ejecucion *contexto, int tam_max_segmento);
#endif