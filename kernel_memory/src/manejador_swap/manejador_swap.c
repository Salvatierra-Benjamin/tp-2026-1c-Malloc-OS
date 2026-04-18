#include "manejador_swap.h"

void *manejador_swap(void *arg)
{
    int socket_swap = *(int *)arg;
    free(arg);

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(socket_swap);
        switch (op_code)
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
        case -1:
        {
            log_warning(logger, "[SWAP] se desconecto swap");
            ejecutando = 0;
            break;
        }
        default:
        {
            log_warning(logger, "[SWAP] operacion desconocida : %d", op_code);
            break;
        }
        }
    }
}

void agregar_swap(int tam_bloque, int tam_swap)
{
    log_debug(logger, "[SWAP] tam_bloque: %d | tam_swap: %d", tam_bloque, tam_swap);
}
