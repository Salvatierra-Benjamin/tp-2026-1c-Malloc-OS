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
            int puerto;
            buffer_read(&puerto, tam_memory_stick->buffer, sizeof(int));
            agregar_memory_stick(tam_ms, puerto, socket_memory_stick);
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

void agregar_memory_stick(int tam_ms, int puerto, int socket_memory_stick)
{
    t_memory_stick *ms = malloc(sizeof(t_memory_stick));
    pthread_mutex_lock(&mutex_lista_memory_sticks);

    if (list_is_empty(lista_memory_sticks))
    {
        ms->base = 0;
    }
    else
    {
        t_memory_stick *ms_ultimo = (t_memory_stick *)list_get(lista_memory_sticks, list_size(lista_memory_sticks) - 1);
        ms->base = ms_ultimo->base + ms_ultimo->tamanio;
    }

    ms->tamanio = tam_ms;
    ms->socket = socket_memory_stick;
    ms->puerto = puerto;

    list_add(lista_memory_sticks, ms);

    pthread_mutex_unlock(&mutex_lista_memory_sticks);

    log_info(logger, "## Memory Stick de %d bytes Conectada", tam_ms);
}
