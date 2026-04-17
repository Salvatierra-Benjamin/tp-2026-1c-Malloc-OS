#ifndef KS_MANEJA_CPU_H
#define KS_MANEJA_CPU_H

#include "ks_estructuras.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <commons/log.h>

#include <../../utils/src/utils/protocolos.h>

// firma correcta para pthread
void* ks_maneja_cpu(void* arg);

#endif