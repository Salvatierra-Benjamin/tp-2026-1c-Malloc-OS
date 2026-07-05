#ifndef KS_MANEJA_IO_H
#define KS_MANEJA_IO_H

#include "../estructuras/ks_estructuras.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <commons/log.h>

#include <../../utils/src/utils/protocolos.h>

// firma correcta para pthread
void* ks_maneja_io(void* arg);

void ks_desbloquear_proceso(t_pcb* pcb);

#endif