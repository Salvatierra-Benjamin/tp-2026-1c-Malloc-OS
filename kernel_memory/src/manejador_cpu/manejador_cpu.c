#include "manejador_cpu.h"

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

            enviar_paquete_a_cpu(cpu_actual, p_ins, socket_cpu);

            destruir_instruccion(instruccion);
            eliminar_paquete(paquete_pid_pc);
            eliminar_paquete(p_ins);
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

    pthread_mutex_lock(&mutex_lista_cpu);
    list_add(lista_cpu, cpu);
    pthread_mutex_unlock(&mutex_lista_cpu);

    log_info(logger, "## CPU %d Conectada", cpu_id);
    return cpu;
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
    copia->PID = original->PID;
    copia->parametro_1 = original->parametro_1 != NULL ? string_duplicate(original->parametro_1) : NULL;
    copia->parametro_2 = original->parametro_2 != NULL ? string_duplicate(original->parametro_2) : NULL;

    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);
    return copia;
}
