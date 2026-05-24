#ifndef KS_MANEJA_CPU_H
#define KS_MANEJA_CPU_H

#include "../estructuras/ks_estructuras.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <commons/log.h>

#include <../../utils/src/utils/protocolos.h>

void* ks_maneja_cpu(void* arg);

#endif