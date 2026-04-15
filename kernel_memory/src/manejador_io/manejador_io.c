#include "manejador_io.h"

void *manejador_io(void *arg)
{
    int socket_io = *(int *)arg;
    free(arg);
}