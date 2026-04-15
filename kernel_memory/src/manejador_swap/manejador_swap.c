#include "manejador_swap.h"

void *manejador_swap(void *arg)
{
    int socket_swap = *(int *)arg;
    free(arg);

    while(1)
    {
        int op_code = recibir_operacion(socket_swap);
        switch(op_code)
        {
            case BLOCK_SIZE:
            {
                t_paquete *block_size_total_swap = recibir_paquete(socket_swap);
                int tam_bloque;
                buffer_read(&tam_bloque, block_size_total_swap->buffer, sizeof(int));
                int tam_swap;
                buffer_read(&tam_swap, block_size_total_swap->buffer, sizeof(int));
                agregar_swap(tam_bloque, tam_swap);
                break;
            }
        }
    }
}

void agregar_swap(int tam_bloque, int tam_swap)
{
    log_debug(logger, "tam_bloque: %d | tam_swap: %d", tam_bloque, tam_swap);
}
