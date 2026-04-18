#include "manejador_memory_stick.h"

void *manejador_memory_stick(void *arg)
{
    int socket_memory_stick = *(int *)arg;
    free(arg);

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(socket_memory_stick);
        switch (op_code)
        {
        case MEMORY_STICK_SIZE:
        {
            t_paquete *tam_memory_stick = recibir_paquete(socket_memory_stick);
            int tam_ms;
            buffer_read(&tam_ms, tam_memory_stick->buffer, sizeof(int));
            agregar_memory_stick(tam_ms);
            eliminar_paquete(tam_memory_stick);
            break;
        }
        case -1:
        {
            log_warning(logger, "[MS] se desconecto memory stick");
            ejecutando = 0;
            break;
        }
        default:
        {
            log_warning(logger, "[MS] operacion desconocida : %d", op_code);
            break;
        }
        }
    }
    close(socket_memory_stick);
    return NULL;
}

void agregar_memory_stick(int tam_ms)
{
    log_info(logger, "## Memory Stick de %d bytes Conectada", tam_ms);
}
