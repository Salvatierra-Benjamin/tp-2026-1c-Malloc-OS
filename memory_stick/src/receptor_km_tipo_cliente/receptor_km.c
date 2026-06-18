#include "receptor_km.h"
// TODO Implementar delay
void receptor_km()
{
    log_debug(ms_logger, "[RECEPTOR_KM_MS] Conectando con KM...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_detach(hilo_atencion_km); // Cuando este hilo termine, que nadie lo espere; libera su memoria automáticamente: pthread_detach(hilo_atencion_km);
    // Si fuera pthread_join (hilo_atencion_km,NULL); dejaria de ser multihilo ya que esta funcion es bloqueante entonces la CPU 2 debe esperar a que termine la CPU 1.
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_km()
{
    log_debug(ms_logger, "[RECEPTOR_KM_MS] CONECTADO CON KM");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(ms_logger, "cod op %d", cod_op);
            log_error(ms_logger, "KM desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_OK:
        {
            log_debug(ms_logger, "[RECEPTOR_KM_MS] EL HANDSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");

            // Conexión a Kernel Memory: “## Conectado a Kernel Memory”
            log_info(ms_logger, "## Conectado a Kernel Memory");

            sem_post(&sem_ok_conexion_km);

            t_paquete *paquete_tamaño_puerto_km = crear_paquete(MEMORY_STICK_SIZE);
            agregar_a_paquete(paquete_tamaño_puerto_km, &tamanio_memoria_ms_ingresada, sizeof(int));
            int puerto = atoi(ms_config.puerto_escucha_cpu);
            agregar_a_paquete(paquete_tamaño_puerto_km, &puerto, sizeof(int));
            enviar_paquete(paquete_tamaño_puerto_km, conexion_km);
            eliminar_paquete(paquete_tamaño_puerto_km);

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
            t_paquete *paquete_peticion_escritura_ms = recibir_paquete(conexion_km);

            // 3. Extracción de datos del buffer (¡Mucha atención al orden en que viajan!)
            // Suponiendo que el Kernel Memory los envió en este orden: PID -> DF -> TAMANIO -> DATOS
            buffer_read(&pid, paquete_peticion_escritura_ms->buffer, sizeof(int));
            buffer_read(&direccion_fisica, paquete_peticion_escritura_ms->buffer, sizeof(uint32_t));
            buffer_read(&tamanio_a_escribir, paquete_peticion_escritura_ms->buffer, sizeof(int));

            // Para los datos variables, primero necesitamos saber el tamaño, reservamos espacio y leemos
            datos = malloc(tamanio_a_escribir);
            buffer_read(datos, paquete_peticion_escritura_ms->buffer, tamanio_a_escribir);
            // Ya no necesitamos el paquete de petición, liberamos
            eliminar_paquete(paquete_peticion_escritura_ms);

            // 4. Procesar la escritura en la memoria física simulada del MS
            procesar_peticion_escritura(conexion_km, direccion_fisica, datos, tamanio_a_escribir);

            // 5. Responder el OK al Kernel Memory incluyendo el PID como control
            t_paquete *paquete_ok_escritura = crear_paquete(RESPUESTA_ESCRITURA_MS);
            agregar_a_paquete(paquete_ok_escritura, &pid, sizeof(int));

            enviar_paquete(paquete_ok_escritura, conexion_km);
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

            // 1. Recibir el paquete enviado por el Kernel Memory
            t_paquete *paquete_peticion_lectura_ms = recibir_paquete(conexion_km);

            // 2. Extraer los datos del buffer
            // Suponiendo que viajan en orden: PID -> DF -> TAMANIO
            buffer_read(&pid, paquete_peticion_lectura_ms->buffer, sizeof(int));
            buffer_read(&direccion_fisica, paquete_peticion_lectura_ms->buffer, sizeof(uint32_t));
            buffer_read(&tamanio_a_leer, paquete_peticion_lectura_ms->buffer, sizeof(int));

            eliminar_paquete(paquete_peticion_lectura_ms);

            // 3. Procesar la lectura
            procesar_peticion_lectura(conexion_km, direccion_fisica, tamanio_a_leer);
            break;
        }
        case KM_OK_CONEXION: // SI ESTO NO ESTA ROMPE (no se porque) NO LO BORRES MAURI!!!
        {
            break;
        }
        default:
        {
            log_warning(ms_logger, "[RECEPTOR_KM_MS] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
