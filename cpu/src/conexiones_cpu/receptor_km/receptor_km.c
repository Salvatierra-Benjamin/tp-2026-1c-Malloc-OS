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
        case CONTEXTO_EJECUCION:
        {
            t_paquete *paquete_contexto = recibir_paquete(conexion_km);

            Recibir_Contexto(&contexto_aux_global, paquete_contexto->buffer);
            Loggear_Contexto(&contexto_aux_global);

            sem_post(&sem_contexto_recibido);
            sem_wait(&sem_contexto_procesado);

            Destruir_Contexto(&contexto_aux_global);

            eliminar_paquete(paquete_contexto);
            break;
        }
        case ERROR_ENVIO_CONTEXTO:
        {
            log_error(cpu_logger, "[CPU] KM reportó que el contexto para PID %d no existe o es NULL.", PID_a_ejecutar);
            exit(EXIT_FAILURE);
            break;
        }
        case ENVIO_INSTRUCCION:
        {
            t_paquete *paquete = recibir_paquete(conexion_km);
            instruccion_aux_global.parametro_1 = NULL;
            instruccion_aux_global.parametro_2 = NULL;

            // 1. Deserializar lo de tamaño fijo (comando y PID)
            buffer_read(&(instruccion_aux_global->comando), paquete->buffer, sizeof(t_palabra_reservada));
            buffer_read(&(instruccion_aux_global->PID), paquete->buffer, sizeof(int));

            // 2. Deserializar parametro_1 (Dinámico)
            int len1;
            buffer_read(&len1, paquete->buffer, sizeof(int)); // Leemos cuánto mide
            // Si len1 es 0 (o menor/igual a 1 si mandan un string vacío ""), no hacemos malloc
            if (len1 <= 1)
            { // Si el tamaño es 0 no hay nada que limpiar en el buffer, por lo tanto lo setea en NULL.
                instruccion_aux_global->parametro_1 = NULL;
                log_warning(cpu_logger, "[CPU] Parámetro 1 no provisto (len=%d). Seteado en NULL.", len1);
                // Si KM igual mandó 1 byte (el \0), avanzamos el buffer para no desfasarnos
                if (len1 == 1)
                {
                    char basura;                              // Reservamos 1 byte de espacio temporal en la pila (stack)
                    buffer_read(&basura, paquete->buffer, 1); // Sacamos ese byte molesto del buffer y lo metemos ahí
                    log_warning(cpu_logger, "[CPU] Parámetro 1 vacío detectado (len=1). Se limpió el byte restante del buffer.");
                    // Al salir de la función, la variable 'basura' desaparece y ese byte se desecha.
                }
            }
            else
            {
                instruccion_aux_global->parametro_1 = malloc(len1);                      // Pedimos el espacio justo
                buffer_read(instruccion_aux_global->parametro_1, paquete->buffer, len1); // Copiamos el contenido
            }

            // 3. Deserializar parametro_2 (Dinámico)
            int len2;
            buffer_read(&len2, paquete->buffer, sizeof(int));
            if (len2 <= 1)
            {
                instruccion_aux_global->parametro_2 = NULL;
                log_warning(cpu_logger, "[CPU] Parámetro 2 no provisto (len=%d). Seteado en NULL.", len2);
                if (len2 == 1)
                {
                    char basura;
                    buffer_read(&basura, paquete->buffer, 1);
                    log_warning(cpu_logger, "[CPU] Parámetro 2 vacío detectado (len=1). Se limpió el byte restante del buffer.");
                }
            }
            else
            {
                instruccion_aux_global->parametro_2 = malloc(len2);
                buffer_read(instruccion_aux_global->parametro_2, paquete->buffer, len2);
            }

            sem_post(&sem_instruccion_recibida);
            sem_wait(&sem_instruccion_procesada);

            Destruir_Instruccion(&instruccion_aux_global);

            eliminar_paquete(paquete);
        }
        case ERROR_ENVIO_INSTRUCCION:
        {
            log_error(cpu_logger, "[CPU] KM notificó un error al intentar obtener la instrucción (¿Fin de archivo sin EXIT?).");

            // Seteamos el comando en un estado de error para que el ciclo de instrucción sepa qué hacer.
            // NO SE LIBERA CON FREE () NADA !!! PUNTEROS APUNTANDO A NULL.
            instruccion_aux_global->comando = ERROR_INSTRUCCION;
            instruccion_aux_global->parametro_1 = NULL;
            instruccion_aux_global->parametro_2 = NULL;
            exit(EXIT_FAILURE);
            break;
        }
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
            log_warning(cpu_logger, "[RECEPTOR_KS_CPU] Código de operación desconocido : %d", cod_op);
            break;
        }
        }
    }
}
