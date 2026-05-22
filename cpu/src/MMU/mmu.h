#ifndef MMU_H
#define MMU_H

#include "gestor_cpu.h"
#include "iniciador_cpu.h"

int MMU_TRADUCCION_FISICA(int dir_logica, uint32_t tam, int *dir_fisica, uint32_t bytes_validos);

#endif