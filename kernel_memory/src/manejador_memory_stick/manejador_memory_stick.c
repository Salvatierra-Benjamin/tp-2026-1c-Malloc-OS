#include "manejador_memory_stick.h"

void *manejador_memory_stick(void *arg)
{
    int socket_memory_stick = *(int *)arg;
    free(arg);
    while (1)
    {
        int op_code = recibir_operacion(socket_memory_stick);
        switch (op_code)
        {
        case HANDSHAKE_MS:
        {
            log_debug(logger, "[MS] ME LLEGO EL HANDSHAKE DE MEMORY STICK : %d", op_code);
            enviar_operacion(socket_memory_stick, MS_OK_CONEXION);
            break;
        }
        case MEMORY_STICK_SIZE:
        {
            t_paquete *tam_memory_stick = recibir_paquete(socket_memory_stick);
            int tam_ms;
            buffer_read(&tam_ms, tam_memory_stick->buffer, sizeof(int));
            agregar_memory_stick(tam_ms);
            break;
        }
        default:
        {
            log_warning(logger, "[MS] operacion desconocida : %d", op_code);
        }
        }
    }
}

void agregar_memory_stick(int tam_ms)
{
    log_info(logger, "## Memory Stick de %d bytes Conectada", tam_ms);
}
