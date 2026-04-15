#include "manejador_ks.h"

void *manejador_ks(void *arg)
{
    int socket_ks = *(int *)arg;
    free(arg);

    log_info(logger, "## Kernel Scheduler Conectado - FD del socket: %d", socket_ks);

    while (1)
    {
        int op_code = recibir_operacion(socket_ks);
        switch (op_code)
        {
        case CREACION_PROCESO:
        {
            t_paquete *info_proceso = recibir_paquete(socket_ks);

            int pid;
            buffer_read(&pid, info_proceso->buffer, sizeof(int));

            int path_size;
            buffer_read(&path_size, info_proceso->buffer, sizeof(int));

            char *path = malloc(path_size);
            buffer_read(path, info_proceso->buffer, path_size);

            creacion_proceso(pid, path);

            free(path);
            eliminar_paquete(info_proceso);
            break;
        }
        }
    }
}

void creacion_proceso(int pid, char *path)
{
    log_debug(logger, "PATH : %s", path);
    log_info(logger, "## PID: %d - Proceso Creado", pid);
}
