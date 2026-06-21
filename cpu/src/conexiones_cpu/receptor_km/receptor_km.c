#include "receptor_km.h"

void receptor_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] Conectando con KS...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_detach(hilo_atencion_km);
}

void *atencion_km()
{
    log_debug(cpu_logger, "[RECEPTOR_KS_CPU] CPU CONECTADO CON KS");

    while (1)
    {

        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(cpu_logger, "cod op %d", cod_op);
            log_error(cpu_logger, "KS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case RECIBIR_MEMORY_STICK:
        {
            t_memory_stick *nuevo_ms = malloc(sizeof(t_memory_stick));
            t_paquete *recibir_MS = recibir_paquete(conexion_km);
            int size_ip, size_puerto;

            buffer_read(&(nuevo_ms->base), recibir_MS->buffer, sizeof(int));
            buffer_read(&(nuevo_ms->tamanio), recibir_MS->buffer, sizeof(int));

            // Desempaquetar IP
            buffer_read(&size_ip, recibir_MS->buffer, sizeof(int));
            nuevo_ms->ip = malloc(size_ip);
            buffer_read(nuevo_ms->ip, recibir_MS->buffer, size_ip);

            // Desempaquetar Puerto
            buffer_read(&size_puerto, recibir_MS->buffer, sizeof(int));
            nuevo_ms->puerto = malloc(size_puerto);
            buffer_read(nuevo_ms->puerto, recibir_MS->buffer, size_puerto);
            eliminar_paquete(recibir_MS);

            list_add(lista_memory_sticks, nuevo_ms);

            // --- CONEXIÓN AL MEMORY STICK ---
            conexion_ms = crear_conexion(nuevo_ms->ip, nuevo_ms->puerto);
            if (conexion_ms == -1)
            {
                log_error(cpu_logger, "ERROR AL CREAR CONEXION CPU->MS.");
                liberar_todo();
                exit(EXIT_FAILURE);
            }

            if (!realizar_handshake(conexion_ms, identificador_cpu, "Memory Stick"))
            {
                liberar_todo();
                exit(EXIT_FAILURE);
            }

            break;
        }
        default:
        {
            log_warning(cpu_logger, "[RECEPTOR_KS_CPU] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
