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
    char *ruta_absoluta = armar_ruta(path);
    t_list *lineas = dividir_archivo_por_lineas(ruta_absoluta);
    free(ruta_absoluta);

    // list_instrucciones = convertir_list_lineas_en_list_instrucciones(list_lineas)
    t_list *instrucciones = convertir_lineas_en_instrucciones(lineas);
    list_destroy_and_destroy_elements(lineas, free);
    
    // contexto_ejecucion = crear_contexto_ejecucion()
    t_procesos_memoria *proceso = malloc(sizeof(t_procesos_memoria));
    proceso->instrucciones = instrucciones;
    proceso->contexto = crear_contexto_ejecucion();

    // crear proceso_memoria y añadir al diccionario (vinculado a pid) (NO OLVIDAD MUTEX)
    char *key = string_itoa(pid);

    pthread_mutex_lock(&mutex_procesos_memoria);
    dictionary_put(procesos_memoria, key, proceso);
    pthread_mutex_unlock(&mutex_procesos_memoria);

    free(key);

    log_debug(logger, "[KS] PATH : %s", path);
    log_info(logger, "## PID: %d - Proceso Creado", pid);

    // pendiente liberacion de memoria aca (instrucciones, procesos)
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

t_list *convertir_lineas_en_instrucciones(t_list *lineas)
{
    t_list_iterator *it = list_iterator_create(lineas);
    t_list *instrucciones = list_create();

    while(list_iterator_has_next(it))
    {
        char *linea = (char*)list_iterator_next(it);
        t_instruccion *instruccion = linea_a_instruccion(linea);
        list_add(instrucciones, instruccion);
    }
    list_iterator_destroy(it);

    return instrucciones;
}

t_instruccion *linea_a_instruccion(char *linea)
{
    char **tokens = string_split(linea, " ");

    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    instruccion->comando = string_a_palabra_reservada(tokens[0]);
    instruccion->parametro_1 = tokens[1] != NULL ? string_duplicate(tokens[1]) : NULL;
    instruccion->parametro_2 = (tokens[1] != NULL && tokens[2] != NULL) ? string_duplicate(tokens[2]) : NULL;

    string_array_destroy(tokens);
    return instruccion;
}

char *palabra_reservada_a_string(t_palabra_reservada comando)
{
    switch (comando)
    {
        case NOOP : return "NOOP";
        case SET : return "SET";
        case MOV_IN : return "MOV_IN";
        case MOV_OUT:      return "MOV_OUT";
        case SUM:          return "SUM";
        case SUB:          return "SUB";
        case JNZ:          return "JNZ";
        case COPY_MEM:     return "COPY_MEM";
        case MUTEX_CREATE: return "MUTEX_CREATE";
        case MUTEX_LOCK:   return "MUTEX_LOCK";
        case MUTEX_UNLOCK: return "MUTEX_UNLOCK";
        case MEM_ALLOC:    return "MEM_ALLOC";
        case MEM_FREE:     return "MEM_FREE";
        case SLEEP:        return "SLEEP";
        case STDOUT:       return "STDOUT";
        case STDIN:        return "STDIN";
        case INIT_PROC:    return "INIT_PROC";
        case EXIT:         return "EXIT";
        default:           return NULL;
    }
}

t_palabra_reservada string_a_palabra_reservada(char *str)
{
    for (t_palabra_reservada cmd = NOOP; cmd <= EXIT; cmd++) {
        char *nombre = palabra_reservada_a_string(cmd);
        if (nombre != NULL && strcmp(str, nombre) == 0) {
            return cmd;
        }
    }
    return -1;
}

t_contexto_ejecucion *crear_contexto_ejecucion()
{
    t_contexto_ejecucion *contexto = calloc(1, sizeof(t_contexto_ejecucion));
    contexto->tabla_segmentos = list_create();
    return contexto;
}
