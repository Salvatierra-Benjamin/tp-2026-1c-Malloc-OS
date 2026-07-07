#include "manejador_ks.h"
#include <manejador_memory_stick/manejador_memory_stick.h>
#include <manejador_swap/manejador_swap.h>

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

// Reincorpora el rango [base, base+tamanio) como hueco libre (ordenado + merge).
static void devolver_hueco(uint32_t base, uint32_t tamanio) {
    t_hueco *h = malloc(sizeof(t_hueco));
    h->base    = base;
    h->tamanio = tamanio;

    pthread_mutex_lock(&mutex_lista_huecos);
    list_add_sorted(lista_huecos, h, (bool (*)(void *, void *))comparar_huecos_por_base);
    mergear_huecos();
    pthread_mutex_unlock(&mutex_lista_huecos);
}

// Elige y consume un hueco según ALLOCATION_STRATEGY. Devuelve la base asignada o
// UINT32_MAX si ningún hueco alcanza; en ese caso *necesita_compactacion indica si
// el espacio total libre alcanzaría (o sea, el problema es la fragmentación).
static uint32_t asignar_espacio(uint32_t tamanio, bool *necesita_compactacion) {
    *necesita_compactacion = false;

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
    return base_asignada;
}

// Traduce una dirección lógica a física para STDIN/STDOUT. Esquema de las pruebas de
// la cátedra: nro_segmento = dir / SEGMENT_MAX_SIZE, offset = dir % SEGMENT_MAX_SIZE.
// Valida que [offset, offset+tamanio) entre en el segmento (si no: segfault lógico).
// La física devuelta es estable: compactación y suspensión corren en este mismo hilo,
// así que nadie mueve segmentos entre la traducción y el acceso.
static bool traducir_direccion_logica(int pid, uint32_t dir_logica, uint32_t tamanio, uint32_t *dir_fisica)
{
    int nro_segmento = dir_logica / parametros->segment_max_size;
    uint32_t offset = dir_logica % parametros->segment_max_size;

    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    free(key);

    if (proceso == NULL)
    {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        return false;
    }

    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;
    segmento_t *seg = NULL;
    for (int i = 0; i < tabla->cantidad_segmentos; i++)
    {
        if (tabla->segmento[i].id_segmento == nro_segmento)
        {
            seg = &tabla->segmento[i];
            break;
        }
    }

    bool valido = (seg != NULL && tamanio > 0 && offset + tamanio <= seg->tam_seg);
    if (valido)
        *dir_fisica = seg->dir_base + offset;

    pthread_mutex_unlock(&mutex_procesos_memoria);
    return valido;
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

            // respuesta [pid]: OK_CREACION_PROCESO o ERROR_CREACION_PROCESO
            // (contrato asumido — KS todavía no implementa el lado receptor)
            // int y no op_code: la variable local 'op_code' del loop sombrea al typedef
            int resultado = creacion_proceso(pid, path, prioridad)
                                ? OK_CREACION_PROCESO
                                : ERROR_CREACION_PROCESO;
            t_paquete *respuesta_creacion = crear_paquete(resultado);
            agregar_a_paquete(respuesta_creacion, &pid, sizeof(int));
            enviar_paquete_a_ks(respuesta_creacion);
            eliminar_paquete(respuesta_creacion);

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
        case SUSPENCION_PROCESO:
        {
            t_paquete *suspender_pid_paquete = recibir_paquete(socket_ks);
            int pid;
            buffer_read(&pid, suspender_pid_paquete->buffer, sizeof(int));
            eliminar_paquete(suspender_pid_paquete);

            if (suspender_proceso(pid))
            {
                t_paquete *ok = crear_paquete(OK_PROCESO_SUSPENDIDO);
                agregar_a_paquete(ok, &pid, sizeof(int));
                enviar_paquete_a_ks(ok);
                eliminar_paquete(ok);
            }
            // PENDIENTE(flujo): no hay opcode de error de suspensión; acordar con KS
            // qué responder si falla (pid inexistente / swap caído).
            // siempre habrá bloques libres suficientes
            break;
        }
        case STDIN_SOLICITUD_ESCRITURA:
        {
            // STDIN: KS ya tiene los datos que IO leyó del teclado; KM los escribe en
            // los sticks. [pid][dir_logica][tamanio][datos] -> OK_STDIN_DATOS_ESCRITOS
            // [pid] o ERROR_STDIN [pid].
            t_paquete *solicitud = recibir_paquete(socket_ks);
            int pid, tamanio;
            uint32_t dir_logica;
            buffer_read(&pid, solicitud->buffer, sizeof(int));
            buffer_read(&dir_logica, solicitud->buffer, sizeof(uint32_t));
            buffer_read(&tamanio, solicitud->buffer, sizeof(int));

            uint32_t dir_fisica;
            bool exito = tamanio > 0 && traducir_direccion_logica(pid, dir_logica, (uint32_t)tamanio, &dir_fisica);

            if (exito)
            {
                void *datos = malloc(tamanio);
                buffer_read(datos, solicitud->buffer, tamanio);
                exito = escribir_rango_global(pid, dir_fisica, (uint32_t)tamanio, datos);
                free(datos);
            }
            eliminar_paquete(solicitud);

            if (!exito)
                log_warning(logger, "[KM] STDIN PID %d: acceso invalido (dir logica %u, tam %d)",
                            pid, dir_logica, tamanio);

            t_paquete *respuesta = crear_paquete(exito ? OK_STDIN_DATOS_ESCRITOS : ERROR_STDIN);
            agregar_a_paquete(respuesta, &pid, sizeof(int));
            enviar_paquete_a_ks(respuesta);
            eliminar_paquete(respuesta);
            break;
        }
        case STDOUT_SOLICITUD_LECTURA:
        {
            // STDOUT: KS pide los datos a imprimir; KM los lee de los sticks.
            // [pid][dir_logica][tamanio] -> STDOUT_RESTUESTA_LECTURA [pid][tamanio][datos]
            // o ERROR_STDOUT [pid].
            t_paquete *solicitud = recibir_paquete(socket_ks);
            int pid, tamanio;
            uint32_t dir_logica;
            buffer_read(&pid, solicitud->buffer, sizeof(int));
            buffer_read(&dir_logica, solicitud->buffer, sizeof(uint32_t));
            buffer_read(&tamanio, solicitud->buffer, sizeof(int));
            eliminar_paquete(solicitud);

            uint32_t dir_fisica;
            void *datos = NULL;
            bool exito = tamanio > 0 && traducir_direccion_logica(pid, dir_logica, (uint32_t)tamanio, &dir_fisica);

            if (exito)
            {
                datos = malloc(tamanio);
                exito = leer_rango_global(pid, dir_fisica, (uint32_t)tamanio, datos);
            }

            if (exito)
            {
                t_paquete *respuesta = crear_paquete(STDOUT_RESTUESTA_LECTURA);
                agregar_a_paquete(respuesta, &pid, sizeof(int));
                agregar_a_paquete(respuesta, &tamanio, sizeof(int));
                agregar_a_paquete(respuesta, datos, tamanio);
                enviar_paquete_a_ks(respuesta);
                eliminar_paquete(respuesta);
            }
            else
            {
                log_warning(logger, "[KM] STDOUT PID %d: acceso invalido (dir logica %u, tam %d)",
                            pid, dir_logica, tamanio);
                t_paquete *error = crear_paquete(ERROR_STDOUT);
                agregar_a_paquete(error, &pid, sizeof(int));
                enviar_paquete_a_ks(error);
                eliminar_paquete(error);
            }
            free(datos);
            break;
        }
        case ENVIO_MEM_FREE:
        {
            // MEM_FREE (PID, SID) — se reusa el opcode CPU->KS, mismo criterio que
            // ENVIO_MEM_ALLOC. Syscall: se responde siempre (OK/ERROR_MEM_FREE [pid]).
            t_paquete *mem_free = recibir_paquete(socket_ks);
            int pid, sid;
            buffer_read(&pid, mem_free->buffer, sizeof(int));
            buffer_read(&sid, mem_free->buffer, sizeof(int));
            eliminar_paquete(mem_free);

            bool exito = eliminar_segmento(pid, sid);
            if (exito)
                log_debug(logger, "[KM] MEM_FREE PID %d: segmento %d liberado", pid, sid);
            else
                log_warning(logger, "[KM] MEM_FREE PID %d SID %d: rechazado (pid o sid inexistente)", pid, sid);

            t_paquete *respuesta = crear_paquete(exito ? OK_MEM_FREE : ERROR_MEM_FREE);
            agregar_a_paquete(respuesta, &pid, sizeof(int));
            enviar_paquete_a_ks(respuesta);
            eliminar_paquete(respuesta);
            break;
        }
        case FIN_PROCESO:
        {
            t_paquete *fin_paquete = recibir_paquete(socket_ks);
            int pid;
            buffer_read(&pid, fin_paquete->buffer, sizeof(int));
            eliminar_paquete(fin_paquete);

            finalizar_proceso(pid);
            break;
        }
        case SOLICITUD_DESSUSPENCION_LISTA:
        {
            // [cantidad][pid1][prioridad1]...[pidN][prioridadN]
            t_paquete *solicitud = recibir_paquete(socket_ks);
            int cantidad;
            buffer_read(&cantidad, solicitud->buffer, sizeof(int));

            int *pids_ok = malloc(cantidad * sizeof(int));
            int *prioridades_ok = malloc(cantidad * sizeof(int));
            int n_ok = 0;

            for (int i = 0; i < cantidad; i++)
            {
                int pid, prioridad;
                buffer_read(&pid, solicitud->buffer, sizeof(int));
                buffer_read(&prioridad, solicitud->buffer, sizeof(int));

                if (dessuspender_proceso(pid, prioridad))
                {
                    pids_ok[n_ok] = pid;
                    prioridades_ok[n_ok] = prioridad;
                    n_ok++;
                }
            }
            eliminar_paquete(solicitud);

            // se responde siempre, aun con cantidad 0: KS necesita saber que el pedido
            // terminó (los que no aparecen siguen suspendidos y se reintentan después)
            t_paquete *respuesta = crear_paquete(RESPUESTA_DESSUSPENCION_LISTA);
            agregar_a_paquete(respuesta, &n_ok, sizeof(int));
            for (int i = 0; i < n_ok; i++)
            {
                agregar_a_paquete(respuesta, &pids_ok[i], sizeof(int));
                agregar_a_paquete(respuesta, &prioridades_ok[i], sizeof(int));
            }
            enviar_paquete_a_ks(respuesta);
            eliminar_paquete(respuesta);

            free(pids_ok);
            free(prioridades_ok);
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

// Devuelve true si el proceso quedó creado. Falla (sin efectos) si el pid ya existe
// (en memoria o suspendido — dictionary_put pisaría el viejo y se perdería) o si el
// script no se puede abrir.
bool creacion_proceso(int pid, char *path, int prioridad)
{
    // guard: pid duplicado en memoria
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    bool ya_existe = dictionary_has_key(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);

    // guard: pid duplicado suspendido (al des-suspender colisionaría en el diccionario)
    if (!ya_existe)
    {
        pthread_mutex_lock(&mutex_lista_procesos_suspendidos);
        for (int i = 0; i < list_size(lista_procesos_suspendidos) && !ya_existe; i++)
        {
            t_proceso_suspendido *ps = list_get(lista_procesos_suspendidos, i);
            ya_existe = (ps->pid == pid);
        }
        pthread_mutex_unlock(&mutex_lista_procesos_suspendidos);
    }

    if (ya_existe)
    {
        free(key);
        log_error(logger, "[KM] CREACION PID %d: ya existe un proceso con ese pid", pid);
        return false;
    }

    char *ruta_absoluta = armar_ruta(path);
    t_list *lineas = dividir_archivo_por_lineas(ruta_absoluta);

    if (lineas == NULL)
    {
        log_error(logger, "[KM] CREACION PID %d: no se pudo abrir el script %s", pid, ruta_absoluta);
        free(ruta_absoluta);
        free(key);
        return false;
    }
    free(ruta_absoluta);

    t_list *instrucciones = convertir_lineas_en_instrucciones(lineas);
    list_destroy_and_destroy_elements(lineas, free);

    t_procesos_memoria *proceso = malloc(sizeof(t_procesos_memoria));
    proceso->instrucciones = instrucciones;
    proceso->contexto = crear_contexto_ejecucion(pid);
    proceso->prioridad = prioridad;

    pthread_mutex_lock(&mutex_procesos_memoria);
    dictionary_put(procesos_memoria, key, proceso);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    log_debug(logger, "[KS] PATH : %s", path);
    log_info(logger, "## PID: %d - Proceso Creado", pid);
    return true;
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

// bloques de swap que ocupa un segmento (cada segmento arranca en bloque nuevo, spec)
static int bloques_para(uint32_t tam_seg)
{
    return (tam_seg + tam_bloque_swap - 1) / tam_bloque_swap;
}

// Suspende un proceso: vuelca todos sus segmentos a SWAP, libera su memoria principal
// y lo mueve de procesos_memoria a lista_procesos_suspendidos. Corre en el hilo de
// manejador_ks. El contenido se escribe a swap ANTES de liberar nada: si algo falla,
// el proceso queda intacto en memoria y se devuelven los bloques reservados.
bool suspender_proceso(int pid)
{
    // fase 1: buscar el proceso (sin sacarlo del diccionario todavía)
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    // el puntero sigue válido fuera del lock: solo este hilo muta o libera procesos
    // (misma garantía documentada en crear_segmento)

    if (proceso == NULL)
    {
        free(key);
        log_warning(logger, "[KM] SUSPENSION PID %d: proceso inexistente", pid);
        return false;
    }

    if (tam_bloque_swap == 0)
    {
        free(key);
        log_warning(logger, "[KM] SUSPENSION PID %d: swap nunca se conecto", pid);
        return false;
    }

    // fase 2: reservar todos los bloques necesarios de una (o nada)
    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;

    int bloques_necesarios = 0;
    for (int i = 0; i < tabla->cantidad_segmentos; i++)
        bloques_necesarios += bloques_para(tabla->segmento[i].tam_seg);

    t_list *bloques = reservar_bloques_swap(bloques_necesarios);
    if (bloques == NULL)
    {
        free(key);
        log_warning(logger, "[KM] SUSPENSION PID %d: sin espacio en swap (%d bloques) | reconfigurar swap para mas espacio", pid, bloques_necesarios);
        return false;
    }

    // fase 3: volcar cada segmento a swap; la memoria queda intacta hasta que todo salga bien
    bool exito = true;
    int bloque_idx = 0;
    for (int i = 0; i < tabla->cantidad_segmentos && exito; i++)
    {
        segmento_t *seg = &tabla->segmento[i];
        int cant = bloques_para(seg->tam_seg);

        // calloc: el último bloque va relleno con ceros hasta tam_bloque_swap
        void *buffer = calloc(cant, tam_bloque_swap);
        exito = leer_rango_global(pid, seg->dir_base, seg->tam_seg, buffer);

        for (int b = 0; b < cant && exito; b++)
        {
            int *nro = list_get(bloques, bloque_idx + b);
            exito = escribir_bloque_swap(*nro, buffer + b * tam_bloque_swap);
            /*
            buffer se guarda de manera contigua
            buffer + b * tam_bloque_swap lee la parte del buffer que entra en el bloque
            */
        }
        free(buffer);
        bloque_idx += cant;
    }

    if (!exito)
    {
        liberar_bloques_swap(bloques);
        free(key);
        log_error(logger, "[KM] SUSPENSION PID %d: fallo el volcado a swap, el proceso queda en memoria", pid);
        return false;
    }

    // fase 4: liberar la memoria principal del proceso
    for (int i = 0; i < tabla->cantidad_segmentos; i++)
        devolver_hueco(tabla->segmento[i].dir_base, tabla->segmento[i].tam_seg);

    // fase 5: mover el proceso del diccionario a la lista de suspendidos
    pthread_mutex_lock(&mutex_procesos_memoria);
    dictionary_remove(procesos_memoria, key); // sin destruir: la metadata se conserva
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    t_proceso_suspendido *suspendido = malloc(sizeof(t_proceso_suspendido));
    suspendido->pid = pid;
    suspendido->proceso = proceso;
    suspendido->bloques = bloques;

    pthread_mutex_lock(&mutex_lista_procesos_suspendidos);
    list_add(lista_procesos_suspendidos, suspendido);
    pthread_mutex_unlock(&mutex_lista_procesos_suspendidos);

    log_debug(logger, "[KM] PID %d suspendido: %d segmentos en %d bloques de swap",
              pid, tabla->cantidad_segmentos, bloques_necesarios);

    return true;
}

// Des-suspende un proceso: asigna memoria nueva a todos sus segmentos, restaura el
// contenido desde SWAP y lo devuelve a procesos_memoria. Espejo de suspender_proceso,
// todo-o-nada: si algún segmento no consigue hueco o falla swap, se devuelven los
// huecos consumidos y el proceso queda suspendido intacto (los bloques NO se liberan:
// el contenido sigue en swap para reintentar cuando KS vuelva a pedirlo).
bool dessuspender_proceso(int pid, int prioridad)
{
    // fase 1: buscar el suspendido (sin sacarlo de la lista todavía)
    pthread_mutex_lock(&mutex_lista_procesos_suspendidos);
    t_proceso_suspendido *suspendido = NULL;
    int idx_suspendido = -1;
    for (int i = 0; i < list_size(lista_procesos_suspendidos); i++)
    {
        t_proceso_suspendido *ps = list_get(lista_procesos_suspendidos, i);
        if (ps->pid == pid)
        {
            suspendido = ps;
            idx_suspendido = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_procesos_suspendidos);
    // puntero e índice válidos fuera del lock: solo este hilo muta la lista

    if (suspendido == NULL)
    {
        log_warning(logger, "[KM] DESSUSPENSION PID %d: no esta suspendido", pid);
        return false;
    }

    tabla_segmentos_t *tabla = &suspendido->proceso->contexto->tabla_segmentos;

    // fase 2: asignar memoria a todos los segmentos. Las bases nuevas van a un array
    // temporal: la tabla no se toca hasta que todo salga bien.
    uint32_t *bases_nuevas = malloc(tabla->cantidad_segmentos * sizeof(uint32_t));
    int asignados = 0;
    bool exito = true;

    for (int i = 0; i < tabla->cantidad_segmentos && exito; i++)
    {
        bool necesita_compactacion = false;
        bases_nuevas[i] = asignar_espacio(tabla->segmento[i].tam_seg, &necesita_compactacion);
        if (bases_nuevas[i] == UINT32_MAX)
        {
            // sin hueco: el proceso sigue suspendido y no aparece en la respuesta a KS.
            // Si fue por fragmentación, una compactación futura (MEM_ALLOC) lo destraba.
            log_warning(logger, "[KM] DESSUSPENSION PID %d: sin espacio para segmento de %u bytes%s",
                        pid, tabla->segmento[i].tam_seg,
                        necesita_compactacion ? " (fragmentacion)" : "");
            exito = false;
        }
        else
        {
            asignados++;
        }
    }

    // fase 3: restaurar el contenido desde swap (bloques en el mismo orden en que
    // suspender_proceso los escribió: cada segmento ocupa sus bloques consecutivos)
    int bloque_idx = 0;
    for (int i = 0; i < tabla->cantidad_segmentos && exito; i++)
    {
        segmento_t *seg = &tabla->segmento[i];
        int cant = bloques_para(seg->tam_seg);

        void *buffer = malloc(cant * tam_bloque_swap);
        for (int b = 0; b < cant && exito; b++)
        {
            int *nro = list_get(suspendido->bloques, bloque_idx + b);
            exito = leer_bloque_swap(*nro, buffer + b * tam_bloque_swap);
        }

        // solo tam_seg bytes: el padding con ceros del último bloque se descarta
        if (exito)
            exito = escribir_rango_global(pid, bases_nuevas[i], seg->tam_seg, buffer);

        free(buffer);
        bloque_idx += cant;
    }

    if (!exito)
    {
        // rollback: devolver los huecos ya consumidos; swap queda intacto
        for (int i = 0; i < asignados; i++)
            devolver_hueco(bases_nuevas[i], tabla->segmento[i].tam_seg);
        free(bases_nuevas);
        log_warning(logger, "[KM] DESSUSPENSION PID %d: fallo, sigue suspendido", pid);
        return false;
    }

    // fase 4: commit — bases nuevas a la tabla (bajo lock: los CPUs copian tablas con él)
    pthread_mutex_lock(&mutex_procesos_memoria);
    for (int i = 0; i < tabla->cantidad_segmentos; i++)
        tabla->segmento[i].dir_base = bases_nuevas[i];
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(bases_nuevas);

    // KS pudo cambiar la prioridad mientras estaba suspendido: manda la vigente
    suspendido->proceso->prioridad = prioridad;

    liberar_bloques_swap(suspendido->bloques);

    pthread_mutex_lock(&mutex_lista_procesos_suspendidos);
    list_remove(lista_procesos_suspendidos, idx_suspendido);
    pthread_mutex_unlock(&mutex_lista_procesos_suspendidos);

    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    dictionary_put(procesos_memoria, key, suspendido->proceso);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    log_debug(logger, "[KM] PID %d des-suspendido: %d segmentos restaurados",
              pid, tabla->cantidad_segmentos);

    free(suspendido);

    return true;
}

// Finaliza un proceso EN MEMORIA: libera sus huecos y destruye su metadata.
// No busca en lista_procesos_suspendidos a propósito: por planificación, EXIT solo
// se alcanza desde EXEC (un suspendido vuelve por SUSP_READY -> READY -> EXEC antes
// de poder terminar), así que un FIN_PROCESO de un pid suspendido sería un bug de KS.
void finalizar_proceso(int pid) {
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = dictionary_get(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (proceso == NULL) {
        log_warning(logger, "[KM] FIN_PROCESO PID %d: no esta en memoria (¿suspendido o inexistente?)", pid);
        return;
    }

    tabla_segmentos_t *tabla = &proceso->contexto->tabla_segmentos;

    // insertar huecos de todos los segmentos
    for (int i = 0; i < tabla->cantidad_segmentos; i++)
        devolver_hueco(tabla->segmento[i].dir_base, tabla->segmento[i].tam_seg);

    // eliminar el proceso del diccionario y liberar memoria
    key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *removido = dictionary_remove(procesos_memoria, key);
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    if (removido != NULL) {
        destruir_proceso_memoria(removido);
    }

    log_debug(logger, "[KM] PID %d finalizado: memoria liberada", pid);
}

// segmentos

// Responde a KS el resultado de un MEM_ALLOC. Siempre responde algo: es una syscall
// y el proceso queda esperando el resultado (KS debe re-despacharlo o finalizarlo).
//   - base != UINT32_MAX -> OK_MEM_ALLOC [pid]
//   - base == UINT32_MAX -> ERROR_MEM_ALLOC [pid] (sin espacio aun tras compactar,
//     tamaño inválido por SEGMENT_MAX_SIZE, o PID/SID inválido)
// El caso "fragmentado" no llega acá: lo intercepta el caller con iniciar_compactacion.
void responder_mem_alloc_a_ks(int pid, uint32_t base) {
    if (base == UINT32_MAX) {
        log_warning(logger, "[KM] MEM_ALLOC PID %d: rechazado, se responde ERROR_MEM_ALLOC", pid);
    }

    t_paquete *respuesta = crear_paquete(base != UINT32_MAX ? OK_MEM_ALLOC : ERROR_MEM_ALLOC);
    agregar_a_paquete(respuesta, &pid, sizeof(int));
    // PENDIENTE(flujo): cuando se defina la actualización de la tabla de segmentos
    // en CPU, acá se agrega también la base (y/o el sid) del segmento nuevo.
    enviar_paquete_a_ks(respuesta);
    eliminar_paquete(respuesta);
}

// Guarda el alloc que no entró por fragmentación y pide a KS que desaloje las CPUs.
// La respuesta (CONFIRMACION_DESALOJO_COMPACTACION) la atiende el loop de manejador_ks.
void iniciar_compactacion(int pid, int sid, uint32_t tamanio) {
    alloc_pendiente.activo = true;
    alloc_pendiente.pid = pid;
    alloc_pendiente.sid = sid;
    alloc_pendiente.tamanio = tamanio;

    log_info(logger, "[KM] MEM_ALLOC PID %d: espacio fragmentado, se solicita desalojo para compactar", pid);

    // Se manda el pid que disparó la compactación: ese proceso está mid-syscall (MEM_ALLOC),
    // su CPU ya lo liberó y quedó idle, así que KS NO debe intentar desalojarlo (una CPU
    // idle no procesa la interrupción ni responde DESALOJO_PROCESO -> KS esperaría la
    // confirmación para siempre). Se re-despacha después vía OK_MEM_ALLOC.
    t_paquete *paquete_desalojo = crear_paquete(DESALOJO_POR_COMPACTACION);
    agregar_a_paquete(paquete_desalojo, &pid, sizeof(int));
    enviar_paquete_a_ks(paquete_desalojo);
    eliminar_paquete(paquete_desalojo);
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

    // guard: tamaño inválido o mayor al máximo configurado (SEGMENT_MAX_SIZE)
    if (tamanio == 0 || tamanio > (uint32_t)parametros->segment_max_size) {
        log_warning(logger, "[KM] MEM_ALLOC PID %d SID %d: tamanio %u invalido (SEGMENT_MAX_SIZE=%d)",
                    pid, sid, tamanio, parametros->segment_max_size);
        return UINT32_MAX;
    }

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

    // fase 2: elegir y consumir hueco
    uint32_t base_asignada = asignar_espacio(tamanio, necesita_compactacion);
    if (base_asignada == UINT32_MAX) {
        return UINT32_MAX;
    }

    // fase 3: escribir la tabla (bajo lock de procesos — los CPUs la copian con este lock)
    pthread_mutex_lock(&mutex_procesos_memoria);

    int nuevo_id = tabla->cantidad_segmentos;
    segmento_t *tmp = realloc(tabla->segmento, (nuevo_id + 1) * sizeof(segmento_t));
    if (tmp == NULL) {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        // devolver el hueco consumido en fase 2: si no, ese rango se pierde para siempre
        devolver_hueco(base_asignada, tamanio);
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

    devolver_hueco(base, tamanio);

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
