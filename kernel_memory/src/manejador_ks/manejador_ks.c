#include "manejador_ks.h"

void *manejador_ks(void *arg)
{
    int socket_ks = *(int *)arg;
    free(arg);
}