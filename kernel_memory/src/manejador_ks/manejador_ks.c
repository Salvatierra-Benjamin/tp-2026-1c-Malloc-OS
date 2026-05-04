#include "manejador_ks.h"

void *manejador_ks(void *arg)
{
    int socket_ks = *(int *)arg;
    free(arg);

    log_info(logger, "## Kernel Scheduler Conectado - FD del socket: %d", socket_ks);

    int ejecutando = 1;

    while (ejecutando)
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
        case -1:
        {
            log_warning(logger, "[KS] se desconecto ks");
            ejecutando = 0;
            break;
        }
        default:
        {
            log_warning(logger, "[KS] operacion desconocida : %d", op_code);
            break;
        }
        }
    }
    close(socket_ks);
    return NULL;
}

void creacion_proceso(int pid, char *path)
{
    // list_lineas = dividir_archivo_por_linea(path_archivo)
    // list_instrucciones = convertir_list_lineas_en_list_instrucciones(list_lineas)
    // contexto_ejecucion = crear_contexto_ejecucion()
    // crear proceso_memoria y añadir al diccionario (vinculado a pid) (NO OLVIDAD MUTEX)
    log_debug(logger, "[KS] PATH : %s", path);
    log_info(logger, "## PID: %d - Proceso Creado", pid);
}

char *armar_ruta(char *ruta_relativa)
{
    return string_from_format("%s/%s", parametros->scripts_basepath, ruta_relativa);
}

t_list *dividir_archivo_por_lineas(char *path)
{
    FILE *pseudocodigo = fopen(path, "r");
    if (pseudocodigo == NULL)
    {
        log_error(logger, "[KS] No se pudo abrir el archivo %s", path);
        return NULL;
    }
    t_list *lineas = list_create();

    char buffer[256];
    while(fgets(buffer, sizeof(buffer), pseudocodigo) != NULL)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (!string_is_empty(buffer))
        {
            list_add(lineas, string_duplicate(buffer));
        }
    }

    fclose(pseudocodigo);

    return lineas;
}