#include "manejador_ks.h"
#include <manejador_memory_stick/manejador_memory_stick.h>

// comparador para list_add_sorted — va en el .h o arriba del .c
static bool comparar_huecos_por_base(t_hueco *a, t_hueco *b) {
    return a->base < b->base;
}

// Alloc que quedó esperando compactación. Solo lo toca el hilo de manejador_ks
// (lo setea iniciar_compactacion y lo consume CONFIRMACION_DESALOJO_COMPACTACION),
// así que no necesita mutex. KS no manda otro MEM_ALLOC mientras compacta.
static struct {
    bool activo;
    int pid;
    int sid;
    uint32_t tamanio;
} alloc_pendiente = { .activo = false };

// referencia a un segmento de un proceso, para ordenar por base al compactar
typedef struct {
    int pid;
    segmento_t *seg;
} t_seg_ref;

static bool comparar_seg_ref_por_base(void *a, void *b) {
    return ((t_seg_ref *)a)->seg->dir_base < ((t_seg_ref *)b)->seg->dir_base;
}

// Envíos a KS chequeando que esté conectado (socket == -1 si nunca conectó o se cayó).
void enviar_operacion_a_ks(op_code operacion) {
    pthread_mutex_lock(&mutex_socket_kernel_scheduler);
    if (socket_kernel_scheduler != -1) {
        enviar_operacion(socket_kernel_scheduler, operacion);
    } else {
        log_warning(logger, "[KM] KS no conectado: se descarta operacion %d", operacion);
    }
    pthread_mutex_unlock(&mutex_socket_kernel_scheduler);
}

void enviar_paquete_a_ks(t_paquete *paquete) {
    pthread_mutex_lock(&mutex_socket_kernel_scheduler);
    if (socket_kernel_scheduler != -1) {
        enviar_paquete(paquete, socket_kernel_scheduler);
    } else {
        log_warning(logger, "[KM] KS no conectado: se descarta paquete %d", paquete->codigo_operacion);
    }
    pthread_mutex_unlock(&mutex_socket_kernel_scheduler);
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
            uint32_t base = crear_segmento(pid, sid, (uint32_t)tamanio, &necesita_compactacion);
            if (base == UINT32_MAX && necesita_compactacion)
            {
                iniciar_compactacion(pid, sid, (uint32_t)tamanio);
                break;
            }
            responder_mem_alloc_a_ks(pid, base);
            break;
        }
        case CONFIRMACION_DESALOJO_COMPACTACION:
        {
            // KS terminó de desalojar todas las CPUs: nadie accede a memoria hasta FIN_COMPACTACION
            compactar();

            if (alloc_pendiente.activo)
            {
                bool necesita_compactacion = false;
                uint32_t base = crear_segmento(alloc_pendiente.pid, alloc_pendiente.sid,
                                               alloc_pendiente.tamanio, &necesita_compactacion);
                // tras compactar el espacio libre es contiguo: si sigue sin entrar, es falta
                // de espacio real — no se reintenta otra compactación
                responder_mem_alloc_a_ks(alloc_pendiente.pid, base);
                alloc_pendiente.activo = false;
            }

            enviar_operacion_a_ks(FIN_COMPACTACION);
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

// Responde a KS el resultado de un MEM_ALLOC:
//   - base != UINT32_MAX -> segmento creado OK
//   - base == UINT32_MAX -> sin espacio (o PID/SID inválido)
// El caso "fragmentado" ya no llega acá: lo intercepta el caller con iniciar_compactacion.
void responder_mem_alloc_a_ks(int pid, uint32_t base) {
    if (base != UINT32_MAX) {
        t_paquete *respuesta = crear_paquete(OK_MEM_ALLOC);
        agregar_a_paquete(respuesta, &pid, sizeof(int));
        // PENDIENTE(flujo): cuando se defina la actualización de la tabla de segmentos
        // en CPU, acá se agrega también la base (y/o el sid) del segmento nuevo.
        enviar_paquete_a_ks(respuesta);
        eliminar_paquete(respuesta);
        return;
    }

    // Sin espacio o PID/SID inválido. Aún no hay opcode de error de alloc en el
    // protocolo; se acuerda con KS al definir el flujo.
    // PENDIENTE(flujo): notificar el fallo a KS con el opcode acordado.
    log_warning(logger, "[KM] MEM_ALLOC PID %d: sin espacio disponible", pid);
}

// Guarda el alloc que no entró por fragmentación y pide a KS que desaloje las CPUs.
// La respuesta (CONFIRMACION_DESALOJO_COMPACTACION) la atiende el loop de manejador_ks.
void iniciar_compactacion(int pid, int sid, uint32_t tamanio) {
    alloc_pendiente.activo = true;
    alloc_pendiente.pid = pid;
    alloc_pendiente.sid = sid;
    alloc_pendiente.tamanio = tamanio;

    log_info(logger, "[KM] MEM_ALLOC PID %d: espacio fragmentado, se solicita desalojo para compactar", pid);

    enviar_operacion_a_ks(DESALOJO_POR_COMPACTACION);
}

// Memoria total = fin del último stick (las bases se asignan contiguas y solo se agregan).
static uint32_t memoria_total(void) {
    pthread_mutex_lock(&mutex_lista_datos_ms);
    uint32_t total = 0;
    if (!list_is_empty(lista_datos_ms)) {
        t_datos_ms *ultimo = list_get(lista_datos_ms, list_size(lista_datos_ms) - 1);
        total = ultimo->base + ultimo->tamanio;
    }
    pthread_mutex_unlock(&mutex_lista_datos_ms);
    return total;
}

// Corre con las CPUs desalojadas (nadie más toca memoria). Desliza todos los segmentos
// de todos los procesos hacia la base 0 en orden de dirección, moviendo los datos reales
// entre sticks, y deja un único hueco al final. Al procesar en orden ascendente, el
// destino de cada segmento nunca pisa datos aún no movidos.
void compactar(void) {
    log_debug(logger, "[KM] inicio de compactacion");

    pthread_mutex_lock(&mutex_procesos_memoria);

    t_list *refs = list_create();
    t_list *procesos = dictionary_elements(procesos_memoria);
    for (int i = 0; i < list_size(procesos); i++) {
        t_procesos_memoria *p = list_get(procesos, i);
        tabla_segmentos_t *tabla = &p->contexto->tabla_segmentos;
        for (int j = 0; j < tabla->cantidad_segmentos; j++) {
            t_seg_ref *ref = malloc(sizeof(t_seg_ref));
            ref->pid = p->contexto->PID;
            ref->seg = &tabla->segmento[j];
            list_add(refs, ref);
        }
    }
    list_destroy(procesos);

    list_sort(refs, comparar_seg_ref_por_base);

    uint32_t cursor = 0;
    bool exito = true;

    for (int i = 0; i < list_size(refs) && exito; i++) {
        t_seg_ref *ref = list_get(refs, i);
        segmento_t *seg = ref->seg;

        if (seg->dir_base != cursor) {
            void *buffer = malloc(seg->tam_seg);
            exito = leer_rango_global(ref->pid, seg->dir_base, seg->tam_seg, buffer)
                 && escribir_rango_global(ref->pid, cursor, seg->tam_seg, buffer);
            free(buffer);

            if (exito) {
                seg->dir_base = cursor;
            }
        }
        cursor += seg->tam_seg;
    }

    list_destroy_and_destroy_elements(refs, free);
    pthread_mutex_unlock(&mutex_procesos_memoria);

    if (!exito) {
        // un read/write a stick solo falla si el stick murió: mismo tratamiento que
        // la desconexión (corrupción -> KS hace BSOD), no se tocan los huecos
        log_error(logger, "[KM] fallo de lectura/escritura durante la compactacion");
        enviar_operacion_a_ks(DETECCION_CORRUPCION_MEMORIA);
        return;
    }

    // todo el espacio libre queda como un único hueco al final
    uint32_t total = memoria_total();
    pthread_mutex_lock(&mutex_lista_huecos);
    list_clean_and_destroy_elements(lista_huecos, free);
    if (cursor < total) {
        t_hueco *h = malloc(sizeof(t_hueco));
        h->base = cursor;
        h->tamanio = total - cursor;
        list_add(lista_huecos, h);
    }
    pthread_mutex_unlock(&mutex_lista_huecos);

    usleep(parametros->compaction_delay * 1000);

    log_debug(logger, "[KM] fin de compactacion: %u bytes ocupados, %u libres contiguos",
              cursor, total - cursor);
}

// UINT32_MAX es error
// La tabla de segmentos se lee (copia) desde los hilos de CPU bajo mutex_procesos_memoria,
// así que toda mutación (realloc/write/memmove) va bajo ese mismo mutex. Los locks de
// procesos y huecos nunca se toman anidados: se trabaja por fases.
uint32_t crear_segmento(int pid, int sid, uint32_t tamanio, bool *necesita_compactacion) {
    *necesita_compactacion = false;

    // fase 1: validar proceso y sid (bajo lock de procesos)
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    free(key);

    if (proceso == NULL) {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        return UINT32_MAX;
    }

    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;
    for (int i = 0; i < tabla->cantidad_segmentos; i++) {
        if (tabla->segmento[i].id_segmento == sid) {
            pthread_mutex_unlock(&mutex_procesos_memoria);
            return UINT32_MAX;  // sid duplicado
        }
    }
    pthread_mutex_unlock(&mutex_procesos_memoria);
    // el puntero 'proceso' y su tabla siguen válidos: el único hilo que los muta o
    // libera es este mismo (manejador_ks); los hilos de CPU solo copian

    // fase 2: elegir y consumir hueco (bajo lock de huecos)
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

    // fase 3: escribir la tabla (bajo lock de procesos — los CPUs la copian con este lock)
    pthread_mutex_lock(&mutex_procesos_memoria);

    int nuevo_id = tabla->cantidad_segmentos;
    segmento_t *tmp = realloc(tabla->segmento, (nuevo_id + 1) * sizeof(segmento_t));
    if (tmp == NULL) {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        // devolver el hueco consumido en fase 2: si no, ese rango se pierde para siempre
        t_hueco *h = malloc(sizeof(t_hueco));
        h->base    = base_asignada;
        h->tamanio = tamanio;
        pthread_mutex_lock(&mutex_lista_huecos);
        list_add_sorted(lista_huecos, h, (bool (*)(void *, void *))comparar_huecos_por_base);
        mergear_huecos();
        pthread_mutex_unlock(&mutex_lista_huecos);
        return UINT32_MAX;
    }
    tabla->segmento = tmp;

    tabla->segmento[nuevo_id].id_segmento = sid;
    tabla->segmento[nuevo_id].dir_base    = base_asignada;
    tabla->segmento[nuevo_id].tam_seg     = tamanio;
    tabla->cantidad_segmentos++;

    pthread_mutex_unlock(&mutex_procesos_memoria);

    // Log obligatorio del enunciado
    log_info(logger, "## PID: %d - Segmento Creado %d - Tamaño: %u", pid, sid, tamanio);

    return base_asignada;
}

bool eliminar_segmento(int pid, int sid) {
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    free(key);

    if (proceso == NULL) {
        pthread_mutex_unlock(&mutex_procesos_memoria);
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
        pthread_mutex_unlock(&mutex_procesos_memoria);
        return false;  // sid no existe
    }

    uint32_t base    = tabla->segmento[idx].dir_base;
    uint32_t tamanio = tabla->segmento[idx].tam_seg;

    // eliminar del array preservando orden (memmove) — bajo lock: los CPUs copian esta tabla
    int restantes = tabla->cantidad_segmentos - idx - 1;
    if (restantes > 0) {
        memmove(&tabla->segmento[idx],
                &tabla->segmento[idx + 1],
                restantes * sizeof(segmento_t));
    }
    tabla->cantidad_segmentos--;

    pthread_mutex_unlock(&mutex_procesos_memoria);

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
