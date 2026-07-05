#include "manejador_memory_stick.h"
#include <manejador_ks/manejador_ks.h>

void *manejador_memory_stick(void *arg)
{
    int socket_memory_stick = *(int *)arg;
    free(arg);

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(socket_memory_stick);
        switch (op_code)
        {
        case MEMORY_STICK_SIZE:
        {
            t_paquete *tam_memory_stick = recibir_paquete(socket_memory_stick);

            /* A.Leer tamaño memoria */
            int tamanio_memoria;
            buffer_read(&tamanio_memoria, tam_memory_stick->buffer, sizeof(int));

            /* B. Leer IP */
            int tamanio_ip;
            buffer_read(&tamanio_ip, tam_memory_stick->buffer, sizeof(int)); // Leemos cuánto mide
            char *ip_recibida = malloc(tamanio_ip);
            buffer_read(ip_recibida, tam_memory_stick->buffer, tamanio_ip); // Leemos el texto

            /* C. Leer Puerto */
            int tamanio_puerto;
            buffer_read(&tamanio_puerto, tam_memory_stick->buffer, sizeof(int));
            char *puerto_recibido = malloc(tamanio_puerto);
            buffer_read(puerto_recibido, tam_memory_stick->buffer, tamanio_puerto);

            /* 2. Usar el string... */
            agregar_memory_stick(tamanio_memoria, ip_recibida, puerto_recibido, socket_memory_stick);

            /* 3. Limpieza de memoria */

            free(ip_recibida);                  // Liberas el espacio del string
            free(puerto_recibido);              // Liberas el espacio del string
            eliminar_paquete(tam_memory_stick); // Liberas el buffer que llegó por la red
            break;
        }
        case -1:
        {
            log_warning(logger, "[MS] se desconecto memory stick");
            ejecutando = 0;
            enviar_operacion_a_ks(DETECCION_CORRUPCION_MEMORIA);
            log_debug(logger, "[MS] corrupcion de memoria enviado a ks");
            break;
        }
        default:
        {
            log_warning(logger, "[MS] operacion desconocida : %d", op_code);
            break;
        }
        }
    }
    close(socket_memory_stick);
    return NULL;
}

void agregar_memory_stick(int tam_ms, char *ip, char *puerto, int socket_memory_stick)
{
    t_memory_stick *ms = malloc(sizeof(t_memory_stick));

    // 1. Guardamos los datos. Usamos string_duplicate para copiar la memoria
    // y asegurarnos de que el char* no se borre si la variable original muere.
    ms->ip = string_duplicate(ip);
    ms->puerto = string_duplicate(puerto);
    ms->tamanio = tam_ms;
    ms->socket = socket_memory_stick;

    // Toda el alta va bajo el mismo lock: si dos sticks conectan a la vez, la base,
    // el hueco y la conexión de datos deben quedar en el mismo orden en las tres
    // listas (memoria_total y mergear_huecos asumen orden ascendente por base).
    pthread_mutex_lock(&mutex_lista_memory_sticks);

    if (list_is_empty(lista_memory_sticks))
    {
        ms->base = 0;
    }
    else
    {
        t_memory_stick *ms_ultimo = (t_memory_stick *)list_get(lista_memory_sticks, list_size(lista_memory_sticks) - 1);
        ms->base = ms_ultimo->base + ms_ultimo->tamanio;
    }

    list_add(lista_memory_sticks, ms);

    abrir_conexion_datos(ms);

    // El espacio del stick recién conectado pasa a estar disponible como hueco libre.
    // Sin esto, lista_huecos queda vacía y crear_segmento nunca encuentra dónde asignar.
    agregar_hueco_por_stick(ms);

    pthread_mutex_unlock(&mutex_lista_memory_sticks);

    aviso_aumento_memoria(ms);

    log_info(logger, "## Memory Stick de %d bytes Conectada", tam_ms);
}

// Abre la conexión de datos dedicada contra el puerto de escucha del stick (el mismo
// que usan las CPUs). Sin handshake: el case HANDSHAKE_CPU del stick no tiene break y
// cae en ENVIO_CPU_ID, que se queda esperando un paquete que nunca llegaría.
// La conexión no se cierra nunca: el stick hace exit(EXIT_FAILURE) si un cliente se va.
void abrir_conexion_datos(t_memory_stick *ms)
{
    int socket_datos = crear_conexion(ms->ip, ms->puerto);
    if (socket_datos < 0)
    {
        log_error(logger, "[MS] no se pudo abrir la conexion de datos con %s:%s", ms->ip, ms->puerto);
        return;
    }

    t_datos_ms *datos = malloc(sizeof(t_datos_ms));
    datos->base = (uint32_t)ms->base;
    datos->tamanio = (uint32_t)ms->tamanio;
    datos->socket_datos = socket_datos;
    pthread_mutex_init(&datos->mutex_datos, NULL);

    pthread_mutex_lock(&mutex_lista_datos_ms);
    list_add(lista_datos_ms, datos);
    pthread_mutex_unlock(&mutex_lista_datos_ms);

    log_debug(logger, "[MS] conexion de datos abierta con %s:%s (base %u, tamanio %u)",
              ms->ip, ms->puerto, datos->base, datos->tamanio);
}

// Devuelve la conexión de datos del stick que contiene la dirección global, o NULL.
// Los sticks nunca se remueven de la lista (desconexión = BSOD), así que el puntero
// devuelto sigue válido después de soltar el mutex de la lista.
static t_datos_ms *datos_ms_para_direccion(uint32_t direccion)
{
    pthread_mutex_lock(&mutex_lista_datos_ms);
    t_list_iterator *it = list_iterator_create(lista_datos_ms);
    t_datos_ms *resultado = NULL;

    while (list_iterator_has_next(it))
    {
        t_datos_ms *datos = list_iterator_next(it);
        if (direccion >= datos->base && direccion < datos->base + datos->tamanio)
        {
            resultado = datos;
            break;
        }
    }

    list_iterator_destroy(it);
    pthread_mutex_unlock(&mutex_lista_datos_ms);
    return resultado;
}

bool leer_rango_global(int pid, uint32_t direccion, uint32_t tamanio, void *destino)
{
    uint32_t leido = 0;

    while (leido < tamanio)
    {
        uint32_t dir_actual = direccion + leido;
        t_datos_ms *datos = datos_ms_para_direccion(dir_actual);
        if (datos == NULL)
        {
            log_error(logger, "[MS] lectura PID %d: direccion global %u fuera de todo stick", pid, dir_actual);
            return false;
        }

        // el pedido no puede cruzar de stick: se parte en el límite y sigue en el próximo
        uint32_t offset_local = dir_actual - datos->base;
        uint32_t restante = tamanio - leido;
        uint32_t disponible_en_stick = datos->tamanio - offset_local;
        int tam_pedido = (int)(restante < disponible_en_stick ? restante : disponible_en_stick);

        pthread_mutex_lock(&datos->mutex_datos);

        t_paquete *peticion = crear_paquete(PETICION_LECTURA_MS);
        agregar_a_paquete(peticion, &pid, sizeof(int));
        agregar_a_paquete(peticion, &offset_local, sizeof(uint32_t));
        agregar_a_paquete(peticion, &tam_pedido, sizeof(int));
        enviar_paquete(peticion, datos->socket_datos);
        eliminar_paquete(peticion);

        int op_code = recibir_operacion(datos->socket_datos);
        if (op_code != RESPUESTA_LECTURA_MS)
        {
            pthread_mutex_unlock(&datos->mutex_datos);
            log_error(logger, "[MS] lectura PID %d: se esperaba RESPUESTA_LECTURA_MS y llego %d", pid, op_code);
            return false;
        }

        t_paquete *respuesta = recibir_paquete(datos->socket_datos);
        buffer_read((uint8_t *)destino + leido, respuesta->buffer, tam_pedido);
        eliminar_paquete(respuesta);

        pthread_mutex_unlock(&datos->mutex_datos);

        leido += tam_pedido;
    }

    // Log obligatorio del enunciado (solo si la operación completa salió bien)
    log_info(logger, "## PID: %d - Lectura - Dir. Física: %u - Tamaño: %u", pid, direccion, tamanio);

    return true;
}

bool escribir_rango_global(int pid, uint32_t direccion, uint32_t tamanio, void *origen)
{
    uint32_t escrito = 0;

    while (escrito < tamanio)
    {
        uint32_t dir_actual = direccion + escrito;
        t_datos_ms *datos = datos_ms_para_direccion(dir_actual);
        if (datos == NULL)
        {
            log_error(logger, "[MS] escritura PID %d: direccion global %u fuera de todo stick", pid, dir_actual);
            return false;
        }

        uint32_t offset_local = dir_actual - datos->base;
        uint32_t restante = tamanio - escrito;
        uint32_t disponible_en_stick = datos->tamanio - offset_local;
        int tam_pedido = (int)(restante < disponible_en_stick ? restante : disponible_en_stick);

        pthread_mutex_lock(&datos->mutex_datos);

        t_paquete *peticion = crear_paquete(PETICION_ESCRITURA_MS);
        agregar_a_paquete(peticion, &pid, sizeof(int));
        agregar_a_paquete(peticion, &offset_local, sizeof(uint32_t));
        agregar_a_paquete(peticion, &tam_pedido, sizeof(int));
        agregar_a_paquete(peticion, (uint8_t *)origen + escrito, tam_pedido);
        enviar_paquete(peticion, datos->socket_datos);
        eliminar_paquete(peticion);

        int op_code = recibir_operacion(datos->socket_datos);
        if (op_code != RESPUESTA_ESCRITURA_MS)
        {
            pthread_mutex_unlock(&datos->mutex_datos);
            log_error(logger, "[MS] escritura PID %d: se esperaba RESPUESTA_ESCRITURA_MS y llego %d", pid, op_code);
            return false;
        }

        t_paquete *respuesta = recibir_paquete(datos->socket_datos);
        int pid_respuesta;
        buffer_read(&pid_respuesta, respuesta->buffer, sizeof(int));
        eliminar_paquete(respuesta);

        pthread_mutex_unlock(&datos->mutex_datos);

        if (pid_respuesta != pid)
        {
            log_error(logger, "[MS] escritura PID %d: el stick confirmo PID %d", pid, pid_respuesta);
            return false;
        }

        escrito += tam_pedido;
    }

    // Log obligatorio del enunciado (solo si la operación completa salió bien)
    log_info(logger, "## PID: %d - Escritura - Dir. Física: %u - Tamaño: %u", pid, direccion, tamanio);

    return true;
}

void aviso_aumento_memoria(t_memory_stick *ms)
{
    // aviso a ks de aumento de memoria
    t_paquete *aviso_ks = crear_paquete(AUMENTO_DE_MEMORIA);
    agregar_a_paquete(aviso_ks, &(ms->tamanio), sizeof(int));
    enviar_paquete_a_ks(aviso_ks);
    eliminar_paquete(aviso_ks);

    pthread_mutex_lock(&mutex_lista_cpu);
    t_list_iterator *it = list_iterator_create(lista_cpu);
    while (list_iterator_has_next(it))
    {
        t_cpu *cpu = (t_cpu *)list_iterator_next(it);
        t_paquete *paquete_ms = crear_paquete(RECIBIR_MEMORY_STICK);

        // Enteros estáticos (base y tamaño)
        agregar_a_paquete(paquete_ms, &(ms->base), sizeof(int));
        agregar_a_paquete(paquete_ms, &(ms->tamanio), sizeof(int));

        // IP y PUERTO (tamaño + string, con el '\0' incluido)
        int ip_size = strlen(ms->ip) + 1;
        agregar_a_paquete(paquete_ms, &ip_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->ip, ip_size);

        int puerto_size = strlen(ms->puerto) + 1;
        agregar_a_paquete(paquete_ms, &puerto_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->puerto, puerto_size);

        // el socket de la CPU también lo escribe su manejador_cpu (respuestas):
        // sin este mutex los paquetes se intercalan y corrompen el stream
        pthread_mutex_lock(&cpu->mutex_socket);
        enviar_paquete(paquete_ms, cpu->cpu_socket);
        pthread_mutex_unlock(&cpu->mutex_socket);

        eliminar_paquete(paquete_ms);
    }
    list_iterator_destroy(it);
    pthread_mutex_unlock(&mutex_lista_cpu);
}
