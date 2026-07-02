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
            pthread_mutex_lock(&mutex_socket_kernel_scheduler);
            enviar_operacion(socket_kernel_scheduler, DETECCION_CORRUPCION_MEMORIA);
            pthread_mutex_unlock(&mutex_socket_kernel_scheduler);
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
    pthread_mutex_unlock(&mutex_lista_memory_sticks);

    // El espacio del stick recién conectado pasa a estar disponible como hueco libre.
    // Sin esto, lista_huecos queda vacía y crear_segmento nunca encuentra dónde asignar.
    agregar_hueco_por_stick(ms);

    aviso_aumento_memoria(ms);

    log_info(logger, "## Memory Stick de %d bytes Conectada", tam_ms);
}

void aviso_aumento_memoria(t_memory_stick *ms)
{
    // aviso a ks de aumento de memoria
    t_paquete *aviso_ks = crear_paquete(AUMENTO_DE_MEMORIA);
    agregar_a_paquete(aviso_ks, &(ms->tamanio), sizeof(int));

    pthread_mutex_lock(&mutex_socket_kernel_scheduler);
    enviar_paquete(aviso_ks, socket_kernel_scheduler);
    pthread_mutex_unlock(&mutex_socket_kernel_scheduler);

    pthread_mutex_lock(&mutex_lista_cpu);
    t_list_iterator *it = list_iterator_create(lista_cpu);
    while (list_iterator_has_next(it))
    {
        t_cpu *cpu = (t_cpu *)list_iterator_next(it);
        t_paquete *paquete_ms = crear_paquete(RECIBIR_MEMORY_STICK);

        // Enteros estáticos (base y tamaño)
        agregar_a_paquete(paquete_ms, &(ms->base), sizeof(int));
        agregar_a_paquete(paquete_ms, &(ms->tamanio), sizeof(int));

        // 3. Empaquetamos la IP (Tamaño + String)
        int ip_size = strlen(ms->ip) + 1; // +1 para enviar el caracter centinela '\0'
        agregar_a_paquete(paquete_ms, &ip_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->ip, ip_size);

        // 4. Empaquetamos el PUERTO (Tamaño + String)
        int puerto_size = strlen(ms->puerto) + 1;
        agregar_a_paquete(paquete_ms, &puerto_size, sizeof(int));
        agregar_a_paquete(paquete_ms, ms->puerto, puerto_size);

        enviar_paquete(paquete_ms, cpu->cpu_socket);
    }
    list_iterator_destroy(it);
    pthread_mutex_unlock(&mutex_lista_cpu);

    eliminar_paquete(aviso_ks);
}
