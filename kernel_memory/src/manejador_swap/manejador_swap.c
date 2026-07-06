#include "manejador_swap.h"

void *manejador_swap(void *arg)
{
    int socket_swap_local = *(int *)arg;
    free(arg);

    // Único mensaje espontáneo de swap tras el handshake: BLOCK_SIZE [tam_bloque][tam_swap]
    int op_code = recibir_operacion(socket_swap_local);
    if (op_code != BLOCK_SIZE)
    {
        log_error(logger, "[SWAP] se esperaba BLOCK_SIZE y llego %d: se descarta la conexion", op_code);
        close(socket_swap_local);
        return NULL;
    }

    t_paquete *block_size_total_swap = recibir_paquete(socket_swap_local);
    int tam_bloque;
    buffer_read(&tam_bloque, block_size_total_swap->buffer, sizeof(int));
    int tam_swap;
    buffer_read(&tam_swap, block_size_total_swap->buffer, sizeof(int));
    eliminar_paquete(block_size_total_swap);

    agregar_swap(tam_bloque, tam_swap);

    // Publicar el socket recién ahora: nadie puede pedir swap sin conocer el tamaño de bloque
    pthread_mutex_lock(&mutex_socket_swap);
    socket_swap = socket_swap_local;
    pthread_mutex_unlock(&mutex_socket_swap);

    // El hilo termina a propósito: todo el tráfico posterior lo inicia KM (suspensión)
    // y es petición/respuesta sincrónica bajo mutex_socket_swap — si este hilo siguiera
    // leyendo, se robaría las respuestas. La caída de swap se detecta al fallar una petición.
    return NULL;
}

void agregar_swap(int tam_bloque, int tam_swap)
{
    if (tam_bloque <= 0)
    {
        log_error(logger, "[SWAP] tamanio de bloque invalido: %d", tam_bloque);
        return;
    }

    pthread_mutex_lock(&mutex_bloques_swap);
    tam_bloque_swap = tam_bloque;
    cantidad_bloques_swap = tam_swap / tam_bloque;
    bloques_swap_ocupados = calloc(cantidad_bloques_swap, sizeof(bool)); // se inicializan en false
    pthread_mutex_unlock(&mutex_bloques_swap);

    log_info(logger, "[SWAP] conectado: %d bloques de %d bytes", cantidad_bloques_swap, tam_bloque);
}

t_list *reservar_bloques_swap(int cantidad)
{
    // metodo de asignacion indexada - lista de bloques correspondientes a un archivo
    t_list *bloques = list_create(); // bloques a reservar : [0,3,5,7] por ejemplo

    pthread_mutex_lock(&mutex_bloques_swap);
    for (int i = 0; i < cantidad_bloques_swap && list_size(bloques) < cantidad; i++)
    {
        if (!bloques_swap_ocupados[i]) // si el bloque esta en false
        {
            bloques_swap_ocupados[i] = true;
            int *nro = malloc(sizeof(int));
            *nro = i;
            // es puntero para que funcione el list add
            list_add(bloques, nro);
        }
    }

    if (list_size(bloques) < cantidad)
    {
        // no alcanzan: deshacer las marcas y no reservar nada
        for (int i = 0; i < list_size(bloques); i++)
        {
            int *nro = list_get(bloques, i);
            bloques_swap_ocupados[*nro] = false;
        }
        pthread_mutex_unlock(&mutex_bloques_swap);
        list_destroy_and_destroy_elements(bloques, free);
        return NULL;
    }

    pthread_mutex_unlock(&mutex_bloques_swap);
    return bloques;
}

void liberar_bloques_swap(t_list *bloques)
{
    pthread_mutex_lock(&mutex_bloques_swap);
    for (int i = 0; i < list_size(bloques); i++)
    {
        int *nro = list_get(bloques, i);
        bloques_swap_ocupados[*nro] = false;
    }
    pthread_mutex_unlock(&mutex_bloques_swap);
    list_destroy_and_destroy_elements(bloques, free);
}

// Marca swap como caído. Llamar con mutex_socket_swap tomado.
static void marcar_swap_caido(void)
{
    log_error(logger, "[SWAP] se desconecto swap");
    close(socket_swap);
    socket_swap = -1;
}

bool escribir_bloque_swap(int bloque, void *datos)
{
    pthread_mutex_lock(&mutex_socket_swap);

    if (socket_swap == -1)
    {
        pthread_mutex_unlock(&mutex_socket_swap);
        log_warning(logger, "[SWAP] escritura de bloque %d descartada: swap no conectado", bloque);
        return false;
    }

    t_paquete *peticion = crear_paquete(PETICION_ESCRITURA_SWAP);
    agregar_a_paquete(peticion, &bloque, sizeof(int));
    agregar_a_paquete(peticion, datos, tam_bloque_swap);
    enviar_paquete(peticion, socket_swap);
    eliminar_paquete(peticion);

    int op_code = recibir_operacion(socket_swap);
    if (op_code != RESPUESTA_ESCRITURA_SWAP)
    {
        if (op_code == -1)
            marcar_swap_caido();
        else
            log_error(logger, "[SWAP] respuesta inesperada a escritura: %d", op_code);
        pthread_mutex_unlock(&mutex_socket_swap);
        return false;
    }

    t_paquete *respuesta = recibir_paquete(socket_swap);
    int bloque_resp, exito;
    buffer_read(&bloque_resp, respuesta->buffer, sizeof(int));
    buffer_read(&exito, respuesta->buffer, sizeof(int));
    eliminar_paquete(respuesta);

    pthread_mutex_unlock(&mutex_socket_swap);

    if (!exito)
        log_error(logger, "[SWAP] swap rechazo la escritura del bloque %d", bloque);

    return exito == 1;
}

bool leer_bloque_swap(int bloque, void *destino)
{
    pthread_mutex_lock(&mutex_socket_swap);

    if (socket_swap == -1)
    {
        pthread_mutex_unlock(&mutex_socket_swap);
        log_warning(logger, "[SWAP] lectura de bloque %d descartada: swap no conectado", bloque);
        return false;
    }

    t_paquete *peticion = crear_paquete(PETICION_LECTURA_SWAP);
    agregar_a_paquete(peticion, &bloque, sizeof(int));
    enviar_paquete(peticion, socket_swap);
    eliminar_paquete(peticion);

    int op_code = recibir_operacion(socket_swap);
    if (op_code != RESPUESTA_LECTURA_SWAP)
    {
        if (op_code == -1)
            marcar_swap_caido();
        else
            log_error(logger, "[SWAP] respuesta inesperada a lectura: %d", op_code);
        pthread_mutex_unlock(&mutex_socket_swap);
        return false;
    }

    t_paquete *respuesta = recibir_paquete(socket_swap);
    int bloque_resp, exito;
    buffer_read(&bloque_resp, respuesta->buffer, sizeof(int));
    buffer_read(&exito, respuesta->buffer, sizeof(int));
    if (exito)
        buffer_read(destino, respuesta->buffer, tam_bloque_swap);
    eliminar_paquete(respuesta);

    pthread_mutex_unlock(&mutex_socket_swap);

    if (!exito)
        log_error(logger, "[SWAP] swap rechazo la lectura del bloque %d", bloque);

    return exito == 1;
}
