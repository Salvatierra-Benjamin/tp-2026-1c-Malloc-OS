#include "manejador_cpu.h"
#include <manejador_ks/manejador_ks.h> // palabra_reservada_a_string

// Envía por el socket de la CPU tomando su mutex si la CPU ya está identificada
// (antes de ENVIO_CPU_ID nadie más conoce este socket, no hay contención).
static void enviar_paquete_a_cpu(t_cpu *cpu, t_paquete *paquete, int socket_cpu)
{
    if (cpu != NULL)
        pthread_mutex_lock(&cpu->mutex_socket);
    enviar_paquete(paquete, socket_cpu);
    if (cpu != NULL)
        pthread_mutex_unlock(&cpu->mutex_socket);
}

static void enviar_operacion_a_cpu(t_cpu *cpu, op_code operacion, int socket_cpu)
{
    if (cpu != NULL)
        pthread_mutex_lock(&cpu->mutex_socket);
    enviar_operacion(socket_cpu, operacion);
    if (cpu != NULL)
        pthread_mutex_unlock(&cpu->mutex_socket);
}

void *manejador_cpu(void *arg)
{
    int socket_cpu = *(int *)arg;
    free(arg);

    t_cpu *cpu_actual = NULL;

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(socket_cpu);
        switch (op_code)
        {
        case ENVIO_CPU_ID:
        {
            t_paquete *paquete_cpu_id = recibir_paquete(socket_cpu);
            int cpu_id;
            buffer_read(&cpu_id, paquete_cpu_id->buffer, sizeof(int));
            cpu_actual = conexion_cpu(cpu_id, socket_cpu);
            eliminar_paquete(paquete_cpu_id);
            break;
        }
        case SOLICITUD_CONTEXTO_EJECUCION:
        {
            t_paquete *paquete_pid = recibir_paquete(socket_cpu);
            int pid;
            buffer_read(&pid, paquete_pid->buffer, sizeof(int));
            t_contexto_ejecucion *contexto = solicitar_contexto(pid);

            if (contexto == NULL)
            {
                enviar_operacion_a_cpu(cpu_actual, ERROR_ENVIO_CONTEXTO, socket_cpu);
                log_error(logger, "[CPU] contexto solicitado para PID %d es NULL", pid);
                eliminar_paquete(paquete_pid);
                break;
            }

            t_paquete *paquete_contexto = crear_paquete(CONTEXTO_EJECUCION);
            agregar_a_paquete(paquete_contexto, &contexto->PID, sizeof(int));
            agregar_a_paquete(paquete_contexto, &contexto->registros, sizeof(t_registros));
            agregar_a_paquete(paquete_contexto, &contexto->tabla_segmentos.cantidad_segmentos, sizeof(uint32_t));
            if (contexto->tabla_segmentos.cantidad_segmentos > 0)
            {
                agregar_a_paquete(paquete_contexto, contexto->tabla_segmentos.segmento, sizeof(segmento_t) * (contexto->tabla_segmentos.cantidad_segmentos));
            }
            agregar_a_paquete(paquete_contexto, &parametros->segment_max_size, sizeof(int));

            enviar_paquete_a_cpu(cpu_actual, paquete_contexto, socket_cpu);

            destruir_contexto(contexto);
            eliminar_paquete(paquete_pid);
            eliminar_paquete(paquete_contexto);
            break;
        }
        case SOLICITUD_INSTRUCCION:
        {
            t_paquete *paquete_pid_pc = recibir_paquete(socket_cpu);
            int pid;
            uint32_t pc;
            buffer_read(&pid, paquete_pid_pc->buffer, sizeof(int));
            buffer_read(&pc, paquete_pid_pc->buffer, sizeof(uint32_t));

            t_instruccion *instruccion = solicitar_instruccion(pid, pc);
            if (instruccion == NULL)
            {
                enviar_operacion_a_cpu(cpu_actual, ERROR_ENVIO_INSTRUCCION, socket_cpu);
                log_error(logger, "[CPU] instruccion solicitada en NULL");
                eliminar_paquete(paquete_pid_pc);
                break;
            }

            t_paquete *p_ins = crear_paquete(ENVIO_INSTRUCCION);
            int comando_int = (int)instruccion->comando;
            agregar_a_paquete(p_ins, &comando_int, sizeof(int));
            agregar_a_paquete(p_ins, &instruccion->PID, sizeof(int));

            // Si el parámetro no existe viaja tamaño 0; si existe vacío viaja 1 (el '\0').
            // La CPU atrapa ambos con len <= 1.
            int parametro_1_size = (instruccion->parametro_1 != NULL) ? (strlen(instruccion->parametro_1) + 1) : 0;
            int parametro_2_size = (instruccion->parametro_2 != NULL) ? (strlen(instruccion->parametro_2) + 1) : 0;

            agregar_a_paquete(p_ins, &parametro_1_size, sizeof(int));
            if (parametro_1_size > 0)
            {
                agregar_a_paquete(p_ins, instruccion->parametro_1, parametro_1_size);
            }
            agregar_a_paquete(p_ins, &parametro_2_size, sizeof(int));
            if (parametro_2_size > 0)
            {
                agregar_a_paquete(p_ins, instruccion->parametro_2, parametro_2_size);
            }

            usleep(parametros->instruction_delay * 1000);

            enviar_paquete_a_cpu(cpu_actual, p_ins, socket_cpu);

            destruir_instruccion(instruccion);
            eliminar_paquete(paquete_pid_pc);
            eliminar_paquete(p_ins);
            break;
        }
        case ACTUALIZAR_CONTEXTO_EJECUCION:
        {
            // CPU manda [pid][t_registros][cantidad_segmentos][segmentos...] en syscall
            // y en desalojo por interrupción. La tabla de segmentos se IGNORA a propósito:
            // la copia de la CPU puede estar vieja (compactación o alloc en paralelo la
            // desactualizan) y la tabla autoritativa es la de KM — acá solo se persisten
            // los registros (PC incluido).
            t_paquete *paquete_ctx = recibir_paquete(socket_cpu);
            int pid;
            t_registros registros;
            buffer_read(&pid, paquete_ctx->buffer, sizeof(int));
            buffer_read(&registros, paquete_ctx->buffer, sizeof(t_registros));
            eliminar_paquete(paquete_ctx);

            actualizar_contexto(pid, &registros);
            break;
        }
        case -1:
        {
            log_warning(logger, "[CPU] se deconecto cpu");
            ejecutando = 0;
            break;
        }
        default:
        {
            log_warning(logger, "[CPU] cod_op desconocido : %d", op_code);
            break;
        }
        }
    }

    // sacar la CPU de la lista antes de cerrar: si queda, aviso_aumento_memoria
    // escribiría a un fd cerrado (o reutilizado por otro cliente)
    if (cpu_actual != NULL)
    {
        pthread_mutex_lock(&mutex_lista_cpu);
        list_remove_element(lista_cpu, cpu_actual);
        pthread_mutex_unlock(&mutex_lista_cpu);
        // nadie más puede tenerla: aviso_aumento_memoria solo la alcanza vía la
        // lista, y toma mutex_lista_cpu durante todo su recorrido
        pthread_mutex_destroy(&cpu_actual->mutex_socket);
        free(cpu_actual);
    }

    close(socket_cpu);
    return NULL;
}

t_cpu *conexion_cpu(int cpu_id, int cpu_socket)
{
    t_cpu *cpu = malloc(sizeof(t_cpu));
    cpu->cpu_id = cpu_id;
    cpu->cpu_socket = cpu_socket;
    pthread_mutex_init(&cpu->mutex_socket, NULL);

    // Orden de locks sticks -> cpu: se toma mutex_lista_memory_sticks durante toda el
    // alta para que un stick que conecta en paralelo (aviso_aumento_memoria) vea la CPU
    // ya publicada en lista_cpu O bien todavía no esté en la lista que le mandamos acá,
    // nunca ambos (evita mandar el stick duplicado) ni ninguno (evita perdérselo).
    // No hay ciclo: la ruta de alta de stick libera este mutex ANTES de tomar
    // mutex_lista_cpu (manejador_memory_stick.c: unlock en :99, aviso en :101).
    pthread_mutex_lock(&mutex_lista_memory_sticks);

    pthread_mutex_lock(&mutex_lista_cpu);
    list_add(lista_cpu, cpu);
    pthread_mutex_unlock(&mutex_lista_cpu);

    // C7: aviso_aumento_memoria solo notifica a las CPUs presentes al conectar un stick
    // nuevo. Una CPU que arranca DESPUÉS de los sticks (orden de arranque típico) no se
    // enteraría de ninguno y encontrar_stick_por_direccion devolvería NULL en el primer
    // MOV_IN/MOV_OUT. Acá le mandamos todos los sticks ya conectados (mismo payload que
    // aviso_aumento_memoria: [base][tamanio][ip_size][ip\0][puerto_size][puerto\0]).
    t_list_iterator *it = list_iterator_create(lista_memory_sticks);
    while (list_iterator_has_next(it))
    {
        t_memory_stick *ms = (t_memory_stick *)list_iterator_next(it);
        t_paquete *paquete_ms = crear_paquete(RECIBIR_MEMORY_STICK);

        agregar_a_paquete(paquete_ms, &(ms->base), sizeof(int));
        agregar_a_paquete(paquete_ms, &(ms->tamanio), sizeof(int));

        int ip_size = strlen(ms->ip) + 1;
        agregar_a_paquete(paquete_ms, &ip_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->ip, ip_size);

        int puerto_size = strlen(ms->puerto) + 1;
        agregar_a_paquete(paquete_ms, &puerto_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->puerto, puerto_size);

        // mismo mutex que usa aviso_aumento_memoria: sin él los paquetes se intercalan
        pthread_mutex_lock(&cpu->mutex_socket);
        enviar_paquete(paquete_ms, cpu->cpu_socket);
        pthread_mutex_unlock(&cpu->mutex_socket);

        eliminar_paquete(paquete_ms);
    }
    list_iterator_destroy(it);

    pthread_mutex_unlock(&mutex_lista_memory_sticks);

    log_info(logger, "## CPU %d Conectada", cpu_id);
    return cpu;
}

// Persiste los registros que devuelve la CPU (syscall o desalojo). El proceso puede
// no existir si KS lo finalizó/suspendió en paralelo: se loguea y se descarta — la
// CPU no espera respuesta.
void actualizar_contexto(int pid, t_registros *registros)
{
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = (t_procesos_memoria *)dictionary_get(procesos_memoria, key);

    if (proceso == NULL)
    {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        free(key);
        log_warning(logger, "[CPU] actualizacion de contexto descartada: PID %d no esta en memoria", pid);
        return;
    }

    proceso->contexto->registros = *registros;
    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    log_debug(logger, "[CPU] contexto actualizado PID %d (PC=%u)", pid, registros->PC);
}

t_contexto_ejecucion *solicitar_contexto(int pid)
{
    log_debug(logger, "[CPU] se solicita contexto : %d", pid);
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);
    t_procesos_memoria *proceso = (t_procesos_memoria *)dictionary_get(procesos_memoria, key);
    if (proceso == NULL)
    {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        free(key);
        log_error(logger, "[CPU] PID %d no existe en memoria", pid);
        return NULL;
    }

    // copia profunda bajo el lock: el hilo de KS puede realloc-ear la tabla
    // (crear_segmento) o liberar el proceso (finalizar_proceso) en paralelo
    t_contexto_ejecucion *copia = malloc(sizeof(t_contexto_ejecucion));
    *copia = *proceso->contexto;
    uint32_t cantidad = proceso->contexto->tabla_segmentos.cantidad_segmentos;
    if (cantidad > 0)
    {
        copia->tabla_segmentos.segmento = malloc(cantidad * sizeof(segmento_t));
        memcpy(copia->tabla_segmentos.segmento,
               proceso->contexto->tabla_segmentos.segmento,
               cantidad * sizeof(segmento_t));
    }
    else
    {
        copia->tabla_segmentos.segmento = NULL;
    }

    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    return copia;
}

t_instruccion *solicitar_instruccion(int pid, uint32_t pc)
{
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);

    t_procesos_memoria *proceso = (t_procesos_memoria *)dictionary_get(procesos_memoria, key);
    if (proceso == NULL)
    {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        free(key);
        log_error(logger, "[CPU] PID %d no existe en memoria", pid);
        return NULL;
    }

    if (pc >= list_size(proceso->instrucciones))
    {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        free(key);
        log_error(logger, "[CPU] PID %d: PC %u fuera de rango", pid, pc);
        return NULL;
    }

    t_instruccion *original = (t_instruccion *)list_get(proceso->instrucciones, pc);

    // copia bajo el lock, mismo motivo que solicitar_contexto
    t_instruccion *copia = malloc(sizeof(t_instruccion));
    copia->comando = original->comando;
    // el PID del original nunca se inicializa (linea_a_instruccion no lo setea):
    // se completa acá con el pid del pedido, que es el único confiable
    copia->PID = pid;
    copia->parametro_1 = original->parametro_1 != NULL ? string_duplicate(original->parametro_1) : NULL;
    copia->parametro_2 = original->parametro_2 != NULL ? string_duplicate(original->parametro_2) : NULL;

    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);

    // Log obligatorio del enunciado (con los argumentos que la instrucción tenga)
    char *args = string_new();
    if (copia->parametro_1 != NULL)
    {
        string_append(&args, " ");
        string_append(&args, copia->parametro_1);
    }
    if (copia->parametro_2 != NULL)
    {
        string_append(&args, " ");
        string_append(&args, copia->parametro_2);
    }
    log_info(logger, "## PID: %d - Obtener instrucción: %u - Instrucción: %s%s",
             pid, pc, palabra_reservada_a_string(copia->comando), args);
    free(args);

    // retardo configurable por pedido de instrucción — fuera de la sección crítica:
    // dentro del lock serializaría a todas las CPUs contra mutex_procesos_memoria
    usleep(parametros->instruction_delay * 1000);

    return copia;
}
