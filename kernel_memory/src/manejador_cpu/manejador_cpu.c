#include "manejador_cpu.h"

void *manejador_cpu(void *arg)
{
    int socket_cpu = *(int *)arg;
    free(arg);

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
            conexion_cpu(cpu_id);
            eliminar_paquete(paquete_cpu_id);
            break;
        }
        case SOLICITUD_CONTEXTO_EJECUCION:
        {
            t_paquete *paquete_pid = recibir_paquete(socket_cpu);
            int pid;
            buffer_read(&pid, paquete_pid->buffer, sizeof(int));
            solicitar_contexto(pid);
            eliminar_paquete(paquete_pid);
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
            //si instruccion == NULL (PID inexistente o pc fuera de rango), responder a CPU con opcode de error en vez de descartar silenciosamente
            if(instruccion == NULL)
            {
                enviar_operacion(socket_cpu, ERROR_ENVIO_INSTRUCCION);
                log_error(logger, "[CPU] instruccion solicitada en NULL");
                break;
            }

            t_paquete *p_ins = crear_paquete(ENVIO_INSTRUCCION);
            agregar_a_paquete(p_ins, instruccion->comando, sizeof(t_palabra_reservada));
            agregar_a_paquete(p_ins, instruccion->PID, sizeof(int));

            int parametro_1_size = strlen(instruccion->parametro_1)+1;
            int parametro_2_size = strlen(instruccion->parametro_2)+1;

            agregar_a_paquete(p_ins, &parametro_1_size, sizeof(int));
            agregar_a_paquete(p_ins, instruccion->parametro_1, parametro_1_size);
            agregar_a_paquete(p_ins, &parametro_2_size, sizeof(int));
            agregar_a_paquete(p_ins, instruccion->parametro_2, parametro_2_size);

            enviar_paquete(p_ins, socket_cpu);

            eliminar_paquete(paquete_pid_pc);
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
    close(socket_cpu);
    return NULL;
}

void conexion_cpu(int cpu_id)
{
    log_info(logger, "## CPU %d Conectada", cpu_id);
}

void solicitar_contexto(int pid)
{
    log_debug(logger, "[CPU] se solicita contexto : %d", pid);
}

t_instruccion *solicitar_instruccion(int pid, uint32_t pc)
{
    char *key = string_itoa(pid);
    pthread_mutex_lock(&mutex_procesos_memoria);

    t_procesos_memoria *proceso = (t_procesos_memoria*)dictionary_get(procesos_memoria, key);
    if (proceso == NULL) {
        pthread_mutex_unlock(&mutex_procesos_memoria);
        free(key);
        log_error(logger, "[CPU] PID %d no existe en memoria", pid);
        return NULL;
    }

    t_instruccion *instruccion = (t_instruccion*)list_get(proceso->instrucciones, pc);

    pthread_mutex_unlock(&mutex_procesos_memoria);
    free(key);
    return instruccion;
}