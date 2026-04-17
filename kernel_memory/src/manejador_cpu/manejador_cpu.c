#include "manejador_cpu.h"

void *manejador_cpu(void *arg)
{
    int socket_cpu = *(int *)arg;
    free(arg);
    while (1)
    {
        int op_code = recibir_operacion(socket_cpu);
        switch (op_code)
        {
        case HANDSHAKE_CPU:
        {
            log_debug(logger, "[CPU] ME LLEGO EL HANDSHAKE DE CPU : %d", op_code);
            enviar_operacion(socket_cpu, KS_OK_CONEXION);
            break;
        }
        case ENVIO_CPU_ID:
        {
            t_paquete *paquete_cpu_id = recibir_paquete(socket_cpu);
            int cpu_id;
            buffer_read(&cpu_id, paquete_cpu_id->buffer, sizeof(int));
            conexion_cpu(cpu_id);
            break;
        }
        case SOLICITUD_CONTEXTO_EJECUCION:
        {
            t_paquete *paquete_pid = recibir_paquete(socket_cpu);
            int pid;
            buffer_read(&pid, paquete_pid->buffer, sizeof(int));
            solicitar_contexto(pid);
            break;
        }
        default:
        {
            log_warning(logger, "[CPU] cod_op desconocido : %d", op_code);
        }
        }
    }
}

void conexion_cpu(int cpu_id)
{
    log_info(logger, "## CPU %d Conectada", cpu_id);
}

void solicitar_contexto(int pid)
{
    log_debug(logger, "[CPU] se solicita contexto : %d", pid);
}
