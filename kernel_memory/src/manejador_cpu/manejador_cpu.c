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