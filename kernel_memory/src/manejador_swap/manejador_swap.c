#include "manejador_swap.h"

void *manejador_swap(void *arg)
{
    int socket_swap = *(int *)arg;
    free(arg);
}