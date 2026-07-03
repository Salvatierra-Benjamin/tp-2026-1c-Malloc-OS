#include "esperar_cpus.h"
void esperar_cpus()
{
    while (1)
    {
        log_debug(ms_logger, "Esperando cpus...");
        int socket_cliente = esperar_cliente(socket_servidor_con_cpu);
        int *socket_cliente_puntero = malloc(sizeof(int));
        *socket_cliente_puntero = socket_cliente;
        pthread_t hilo_atencion_cpus;
        pthread_create(&hilo_atencion_cpus, NULL, atencion_cpus, (void *)socket_cliente_puntero);
        pthread_detach(hilo_atencion_cpus); // Cuando este hilo termine, que nadie lo espere; libera su memoria automáticamente.
        // Si fuera pthread_join (hilo_atencion_km,NULL); dejaria de ser multihilo ya que esta funcion es bloqueante entonces la CPU 2 debe esperar a que termine la CPU 1.
        // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
        // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
    }
}

void *atencion_cpus(void *socket_cliente_puntero)
{
    log_debug(ms_logger, "Se conecto un CPU");
    int socket_cliente = *((int *)(socket_cliente_puntero));
    free(socket_cliente_puntero);

    while (1)
    {

        int cod_op = recibir_operacion(socket_cliente);

        if (cod_op < 0)
        {
            log_error(ms_logger, "cod op %d", cod_op);
            log_error(ms_logger, "CPU desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_CPU:
        {
            log_debug(ms_logger, "[RECEPTOR_CPUS]HANDSHAKE EXITOSO CON CPU!!!");
            enviar_operacion(socket_cliente, HANDSHAKE_OK);
        }
        case ENVIO_CPU_ID:
        {
            t_paquete *paquete_cpu_id = recibir_paquete(socket_cliente);
            int cpu_id;
            buffer_read(&cpu_id, paquete_cpu_id->buffer, sizeof(int));
            eliminar_paquete(paquete_cpu_id);
            log_debug(ms_logger, "[RECEPTOR_CPUS]Recibido el cpu_id número: %d", cpu_id);

            // Conexión de CPU: “## CPU <ID CPU> Conectada”
            log_info(ms_logger, "“## CPU %d Conectada”", cpu_id);
            break;
        }
        case PETICION_ESCRITURA_MS:
        {
            // 1. Declarar las variables donde guardaremos lo recibido
            int pid;
            uint32_t direccion_fisica;
            int tamanio_a_escribir;
            void *datos = NULL; // Lo dejamos en NULL para inicializarlo con malloc dinámicamente

            // 2. Recibir el paquete (Corregido el tipo de dato: era t_paquete*, no int t_paquete*)
            t_paquete *paquete_peticion_escritura_ms = recibir_paquete(socket_cliente);

            // 3. Extracción de datos del buffer (¡Mucha atención al orden en que viajan!)
            // Suponiendo que el socket_cliente los envió en este orden: PID -> DF -> TAMANIO -> DATOS
            buffer_read(&pid, paquete_peticion_escritura_ms->buffer, sizeof(int));
            buffer_read(&direccion_fisica, paquete_peticion_escritura_ms->buffer, sizeof(uint32_t));
            buffer_read(&tamanio_a_escribir, paquete_peticion_escritura_ms->buffer, sizeof(int));

            // Para los datos variables, primero necesitamos saber el tamaño, reservamos espacio y leemos
            datos = malloc(tamanio_a_escribir);
            buffer_read(datos, paquete_peticion_escritura_ms->buffer, tamanio_a_escribir);
            // Ya no necesitamos el paquete de petición, liberamos
            eliminar_paquete(paquete_peticion_escritura_ms);

            // 4. Procesar la escritura en la memoria física simulada del MS
            pthread_mutex_lock(&mutex_escritura);
            procesar_peticion_escritura(socket_cliente, direccion_fisica, datos, tamanio_a_escribir);
            pthread_mutex_unlock(&mutex_escritura);
            // 5. Responder el OK al socket_cliente incluyendo el PID como control
            t_paquete *paquete_ok_escritura = crear_paquete(RESPUESTA_ESCRITURA_MS);
            agregar_a_paquete(paquete_ok_escritura, &pid, sizeof(int));

            enviar_paquete(paquete_ok_escritura, socket_cliente);

            // Escritura: “## Escritura de <CANTIDAD ESCRITA> bytes”
            log_info(ms_logger, "## Escritura de %d bytes", tamanio_a_escribir);

            // 6. Limpieza absoluta para evitar memory leaks
            eliminar_paquete(paquete_ok_escritura);
            free(datos);
            break;
        }
        case PETICION_LECTURA_MS:
        {
            int pid;
            uint32_t direccion_fisica;
            int tamanio_a_leer;

            // 1. Recibir el paquete enviado por el socket_cliente
            t_paquete *paquete_peticion_lectura_ms = recibir_paquete(socket_cliente);

            // 2. Extraer los datos del buffer
            // Suponiendo que viajan en orden: PID -> DF -> TAMANIO
            buffer_read(&pid, paquete_peticion_lectura_ms->buffer, sizeof(int));
            buffer_read(&direccion_fisica, paquete_peticion_lectura_ms->buffer, sizeof(uint32_t));
            buffer_read(&tamanio_a_leer, paquete_peticion_lectura_ms->buffer, sizeof(int));

            eliminar_paquete(paquete_peticion_lectura_ms);

            // 3. Procesar la lectura
            pthread_mutex_lock(&mutex_lecutra);
            procesar_peticion_lectura(socket_cliente, direccion_fisica, tamanio_a_leer);
            pthread_mutex_unlock(&mutex_lecutra);
            // Lectura: “## Lectura de <CANTIDAD ESCRITA> bytes” // Asumno que se equivocaron en el enunciado
            log_info(ms_logger, "## Lectura de %d bytes", tamanio_a_leer);
            break;
        }
        default:
        {
            log_warning(ms_logger, "[RECEPTOR_CPUS]Operacion Desconocida: %d", cod_op);
            break;
        }
        }
    }
}
