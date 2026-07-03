#include "receptor_km.h"
#include "../operaciones/operaciones.h"

// Contrato de respuestas SWAP -> KM (KM todavía no lo consume; al implementarlo
// del lado de KM respetar este formato):
//   RESPUESTA_ESCRITURA_SWAP: [bloque: int][exito: int (1 ok / 0 error)]
//   RESPUESTA_LECTURA_SWAP:   [bloque: int][exito: int][datos: BLOCK_SIZE bytes, solo si exito]
void receptor_km()
{
    log_debug(swap_logger, "[RECEPTOR_KM_SWAP] Conectando con KM...");
    pthread_t hilo_atencion_km;
    pthread_create(&hilo_atencion_km, NULL, atencion_km, NULL);
    pthread_join(hilo_atencion_km, NULL);
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de KM y servidor de CPU al mismo tiempo).
}

void *atencion_km(void *arg)
{
    log_debug(swap_logger, "[RECEPTOR_KM_SWAP] CONECTADO CON KM");

    while (1)
    {
        int cod_op = recibir_operacion(conexion_km);

        if (cod_op < 0)
        {
            log_error(swap_logger, "cod op %d", cod_op);
            log_error(swap_logger, "KM desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_OK:
        {
            log_debug(swap_logger, "[RECEPTOR_KM_SWAP] EL HANSHAKE CON KM SE HA REALIZADO CORRECTAMENTE!!!");
            // Conexión a Kernel Memory: “## Conectado a Kernel Memory”
            log_info(swap_logger, "## Conectado a Kernel Memory");
            // Enviar tamaño de bloque y tamaño total de SWAP al KM
            t_paquete *paquete_block = crear_paquete(BLOCK_SIZE);
            int tam_bloque = swap_config.swap_block_size;
            int tam_swap = swap_config.swap_file_size;
            agregar_a_paquete(paquete_block, &tam_bloque, sizeof(int));
            agregar_a_paquete(paquete_block, &tam_swap, sizeof(int));
            enviar_paquete(paquete_block, conexion_km);
            eliminar_paquete(paquete_block);
            break;
        }
        case PETICION_ESCRITURA_SWAP:
        {
            t_paquete *paquete_peticion = recibir_paquete(conexion_km);
            int bloque;
            buffer_read(&bloque, paquete_peticion->buffer, sizeof(int));

            // El payload tiene que traer el bloque + exactamente BLOCK_SIZE bytes de datos
            int tamanio_esperado = sizeof(int) + swap_config.swap_block_size;
            int exito = 0;
            if (paquete_peticion->buffer->size < tamanio_esperado)
            {
                log_error(swap_logger, "Escritura rechazada: payload de %d bytes (se esperaban %d) para el bloque %d",
                          paquete_peticion->buffer->size, tamanio_esperado, bloque);
            }
            else
            {
                void *datos = malloc(swap_config.swap_block_size);
                buffer_read(datos, paquete_peticion->buffer, swap_config.swap_block_size);

                pthread_mutex_lock(&mutex_archivo_swap);
                exito = escribir_bloque(bloque, datos) > 0;
                pthread_mutex_unlock(&mutex_archivo_swap);
                free(datos);
            }
            eliminar_paquete(paquete_peticion);

            t_paquete *paquete_resp = crear_paquete(RESPUESTA_ESCRITURA_SWAP);
            agregar_a_paquete(paquete_resp, &bloque, sizeof(int));
            agregar_a_paquete(paquete_resp, &exito, sizeof(int));
            enviar_paquete(paquete_resp, conexion_km);
            eliminar_paquete(paquete_resp);

            if (exito)
            {
                log_info(swap_logger, "## Escritura del bloque: %d", bloque);
            }
            break;
        }
        case PETICION_LECTURA_SWAP:
        {
            t_paquete *paquete_peticion = recibir_paquete(conexion_km);
            int bloque;
            buffer_read(&bloque, paquete_peticion->buffer, sizeof(int));
            eliminar_paquete(paquete_peticion);

            void *buffer = malloc(swap_config.swap_block_size);
            pthread_mutex_lock(&mutex_archivo_swap);
            int exito = leer_bloque(bloque, buffer) > 0;
            pthread_mutex_unlock(&mutex_archivo_swap);

            t_paquete *paquete_resp = crear_paquete(RESPUESTA_LECTURA_SWAP);
            agregar_a_paquete(paquete_resp, &bloque, sizeof(int));
            agregar_a_paquete(paquete_resp, &exito, sizeof(int));
            if (exito)
            {
                agregar_a_paquete(paquete_resp, buffer, swap_config.swap_block_size);
            }
            enviar_paquete(paquete_resp, conexion_km);
            eliminar_paquete(paquete_resp);
            free(buffer);

            if (exito)
            {
                log_info(swap_logger, "## Lectura del bloque: %d", bloque);
            }
            break;
        }
        default:
        {
            log_warning(swap_logger, "[RECEPTOR_KM_SWAP] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
