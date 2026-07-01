#include "manejador_ks.h"

// comparador para list_add_sorted — va en el .h o arriba del .c
static bool comparar_huecos_por_base(t_hueco *a, t_hueco *b) {
    return a->base < b->base;
}

static t_hueco *best_fit(uint32_t tamanio) {
    t_list_iterator *it = list_iterator_create(lista_huecos);
    t_hueco *resultado = NULL;

    while (list_iterator_has_next(it)) {
        t_hueco *h = list_iterator_next(it);
        if (h->tamanio >= tamanio) {
            if (resultado == NULL || h->tamanio < resultado->tamanio) {
                resultado = h;
            }
        }
    }

    list_iterator_destroy(it);
    return resultado;
}

// worst_fit — llamar con mutex_lista_huecos tomado
static t_hueco *worst_fit(uint32_t tamanio) {
    t_list_iterator *it = list_iterator_create(lista_huecos);
    t_hueco *resultado = NULL;

    while (list_iterator_has_next(it)) {
        t_hueco *h = list_iterator_next(it);
        if (h->tamanio >= tamanio) {
            if (resultado == NULL || h->tamanio > resultado->tamanio) {
                resultado = h;
            }
        }
    }

    list_iterator_destroy(it);
    return resultado;
}

static uint32_t espacio_total_libre(void) {
    uint32_t total = 0;
    t_list_iterator *it = list_iterator_create(lista_huecos);

    while (list_iterator_has_next(it)) {
        t_hueco *h = list_iterator_next(it);
        total += h->tamanio;
    }

    list_iterator_destroy(it);
    return total;
}

void *manejador_ks(void *arg)
{
    int socket_ks = *(int *)arg;
    free(arg);

    log_info(logger, "## Kernel Scheduler Conectado - FD del socket: %d", socket_ks);

    int ejecutando = 1;

    pthread_mutex_lock(&mutex_socket_kernel_scheduler);
    socket_kernel_scheduler = socket_ks;
    pthread_mutex_unlock(&mutex_socket_kernel_scheduler);

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

            int prioridad;
            buffer_read(&prioridad, info_proceso->buffer, sizeof(int));

            creacion_proceso(pid, path, prioridad);

            free(path);
            eliminar_paquete(info_proceso);
            break;
        }
        case ENVIO_MEM_ALLOC:
        {
            // MEM_ALLOC (PID, SID, TAMAÑO)
            int pid, sid, tamanio;
            t_paquete *mem_alloc = recibir_paquete(socket_ks);
            buffer_read(&pid, mem_alloc->buffer, sizeof(int));
            buffer_read(&sid, mem_alloc->buffer, sizeof(int));
            buffer_read(&tamanio, mem_alloc->buffer, sizeof(int));
            eliminar_paquete(mem_alloc);
            log_debug(logger, "pid : %d | sid : %d | tamanio : %d", pid, sid, tamanio);
            bool necesita_compactacion = false;
            crear_segmento(pid, sid, (uint32_t)tamanio, &necesita_compactacion);
            break;
        }
        case -1:
        {
            log_warning(logger, "[KS] se desconecto ks");
            ejecutando = 0;
            pthread_mutex_lock(&mutex_socket_kernel_scheduler);
            socket_kernel_scheduler = -1;
            pthread_mutex_unlock(&mutex_socket_kernel_scheduler);
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

void creacion_proceso(int pid, char *path, int prioridad)
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
    proceso->contexto = crear_contexto_ejecucion(pid);
    proceso->prioridad = prioridad;

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

t_contexto_ejecucion *crear_contexto_ejecucion(int pid)
{
    t_contexto_ejecucion *contexto = calloc(1, sizeof(t_contexto_ejecucion));
    contexto->PID = pid;
    contexto->tabla_segmentos.segmento = NULL;
    contexto->tabla_segmentos.cantidad_segmentos = 0;
    return contexto;
}

// finalizar proceso
void finalizar_proceso(int pid) {
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (proceso == NULL) {
        return;
    }

    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;

    // insertar huecos de todos los segmentos
    pthread_mutex_lock(&mutex_lista_huecos);
    for (int i = 0; i < tabla->cantidad_segmentos; i++) 
    {
        t_hueco *h = malloc(sizeof(t_hueco));
        h->base    = tabla->segmento[i].dir_base;
        h->tamanio = tabla->segmento[i].tam_seg;
        list_add_sorted(lista_huecos, h,
            (bool(*)(void*, void*)) comparar_huecos_por_base);
    }
    mergear_huecos();  // una sola vez al final
    pthread_mutex_unlock(&mutex_lista_huecos);

    // eliminar el proceso del diccionario y liberar memoria
    key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *removido = dictionary_remove(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (removido != NULL) {
        destruir_proceso_memoria(removido);
    }
}

// segmentos

// UINT32_MAX es error
uint32_t crear_segmento(int pid, int sid, uint32_t tamanio, bool *necesita_compactacion) {
    *necesita_compactacion = false;

    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (proceso == NULL) {
        return UINT32_MAX;
    }

    // verificar que el sid no exista ya
    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;
    for (int i = 0; i < tabla->cantidad_segmentos; i++) {
        if (tabla->segmento[i].id_segmento == sid) {
            return UINT32_MAX;  // sid duplicado
        }
    }

    pthread_mutex_lock(&mutex_lista_huecos);

    t_hueco *hueco = NULL;
    if (strcmp(parametros->allocation_strategy, "BEST") == 0) {
        hueco = best_fit(tamanio);
    } else {
        hueco = worst_fit(tamanio);
    }

    if (hueco == NULL) {
        if (espacio_total_libre() >= tamanio) {
            *necesita_compactacion = true;
        }
        pthread_mutex_unlock(&mutex_lista_huecos);
        return UINT32_MAX;
    }

    uint32_t base_asignada = hueco->base;

    if (hueco->tamanio == tamanio) {
        t_list_iterator *it = list_iterator_create(lista_huecos);
        while (list_iterator_has_next(it)) {
            t_hueco *h = list_iterator_next(it);
            if (h->base == base_asignada) {
                list_iterator_remove(it);
                free(h);
                break;
            }
        }
        list_iterator_destroy(it);
    } else {
        hueco->base    += tamanio;
        hueco->tamanio -= tamanio;
    }

    pthread_mutex_unlock(&mutex_lista_huecos);

    int nuevo_id = tabla->cantidad_segmentos;
    segmento_t *tmp = realloc(tabla->segmento, (nuevo_id + 1) * sizeof(segmento_t));
    if (tmp == NULL) {
        return UINT32_MAX;
    }
    tabla->segmento = tmp;

    tabla->segmento[nuevo_id].id_segmento = sid;
    tabla->segmento[nuevo_id].dir_base    = base_asignada;
    tabla->segmento[nuevo_id].tam_seg     = tamanio;
    tabla->cantidad_segmentos++;

    return base_asignada;
}

bool eliminar_segmento(int pid, int sid) {
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (proceso == NULL) {
        return false;
    }

    // buscar el segmento por sid
    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;
    int idx = -1;
    for (int i = 0; i < tabla->cantidad_segmentos; i++) {
        if (tabla->segmento[i].id_segmento == sid) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        return false;  // sid no existe
    }

    uint32_t base    = tabla->segmento[idx].dir_base;
    uint32_t tamanio = tabla->segmento[idx].tam_seg;

    // eliminar del array preservando orden (memmove)
    int restantes = tabla->cantidad_segmentos - idx - 1;
    if (restantes > 0) {
        memmove(&tabla->segmento[idx],
                &tabla->segmento[idx + 1],
                restantes * sizeof(segmento_t));
    }
    tabla->cantidad_segmentos--;

    // liberar el hueco
    t_hueco *h = malloc(sizeof(t_hueco));
    h->base    = base;
    h->tamanio = tamanio;

    pthread_mutex_lock(&mutex_lista_huecos);
    list_add_sorted(lista_huecos, h, (bool(*)(void*, void*)) comparar_huecos_por_base);
    mergear_huecos();
    pthread_mutex_unlock(&mutex_lista_huecos);

    return true;
}

// huecos
void mergear_huecos(void)
{
    t_list_iterator *it = list_iterator_create(lista_huecos);
    t_hueco *actual = NULL;

    while (list_iterator_has_next(it)) {
        t_hueco *siguiente = (t_hueco*)list_iterator_next(it);

        if (actual != NULL) {
            if (actual->base + actual->tamanio == siguiente->base) {
                // son contiguos — absorber siguiente en actual
                actual->tamanio += siguiente->tamanio;
                list_iterator_remove(it);
                free(siguiente);
                // no pisamos actual — el nuevo siguiente puede ser también contiguo
                continue;
            }
        }

        actual = siguiente;
    }

    list_iterator_destroy(it);
}

void agregar_hueco_por_stick(t_memory_stick *stick)
{
    t_hueco *h = malloc(sizeof(t_hueco));
    h->base    = (uint32_t)stick->base;
    h->tamanio = (uint32_t)stick->tamanio;

    pthread_mutex_lock(&mutex_lista_huecos);
    list_add(lista_huecos, h);  // siempre va al final — mayor base
    mergear_huecos();
    pthread_mutex_unlock(&mutex_lista_huecos);
}
