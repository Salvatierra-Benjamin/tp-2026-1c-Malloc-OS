#include "manejador_swap.h"

void *manejador_swap(void *arg)
{
    int socket_swap_local = *(int *)arg;
    free(arg);

    pthread_mutex_lock(&mutex_socket_swap);
    socket_swap = socket_swap_local;
    pthread_mutex_unlock(&mutex_socket_swap);

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(socket_swap_local);
        switch (op_code)
        {
        case BLOCK_SIZE:
        {
            t_paquete *block_size_total_swap = recibir_paquete(socket_swap_local);
            int tam_bloque;
            buffer_read(&tam_bloque, block_size_total_swap->buffer, sizeof(int));
            int tam_swap;
            buffer_read(&tam_swap, block_size_total_swap->buffer, sizeof(int));
            agregar_swap(tam_bloque, tam_swap);
            eliminar_paquete(block_size_total_swap);
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
    close(socket_swap_local);
    return NULL;
}

void agregar_swap(int tam_bloque, int tam_swap)
{
    log_debug(logger, "[SWAP] tam_bloque: %d | tam_swap: %d", tam_bloque, tam_swap);
}
