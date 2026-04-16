// #include <swap_utils.h>

// //! Funciones_Conexion:
// void *atencion_clientes_de_storage(void *socket_cliente) //* Se encarga de atender a los Workers, procesa la informacion y envia respuesta.
// {
//     log_debug(logger, "se conecto un cliente");
//     // int *fd = (int *)(socket_cliente);
//     // int socket = (int)*fd;
//     int socket = *((int *)(socket_cliente));
//     free(socket_cliente);

//     while (1)
//     {
//         int cod_op = recibir_operacion(socket);

//         switch (cod_op)
//         {
//         case W_ENVIO_ID:
//         {
//             char *id_worker;
//             recibir_mensaje(socket, &id_worker);
//             log_debug(logger, "Se conecto el worker con id: %s", id_worker);
//             free(id_worker); // CUANTO TERMINA DE USARLO FREE
//             break;
//         }
//         case PEDIDO_TAMANIO_BLOQUE_WORKER_STORAGE:
//         {
//             char *pedido_worker_tamanio;
//             char *block_size;
//             block_size = config_get_string_value(config_super_block, "BLOCK_SIZE");
//             recibir_mensaje(socket, &pedido_worker_tamanio);
//             log_debug(logger, "%s", pedido_worker_tamanio);
//             free(pedido_worker_tamanio);
//             // RESPONDER CON EL MOCK OBJECT;
//             enviar_mensaje(block_size, socket, RECIBIDOR_PEDIDO_TAMANIO_BLOQUE_WORKER_STORAGE);
//             break;
//         }
//         case ENVIO_CREATE:
//         {
//             t_list *partes_create;
//             recibir_paquete(socket, &partes_create);
//             Operacion_Create(partes_create);

//             char *file_tag = list_get(partes_create, 0); // FILE_TAG
//             char *file = list_get(partes_create, 1);     // FILE
//             char *tag = list_get(partes_create, 2);      // TAG
//             // char *query_id = list_get(partes_create, 3);
//             log_debug(logger, "Recibi el FILE:TAG %s", file_tag);
//             log_debug(logger, "## Separados: %s:%s", file, tag);
//             // log_debug(logger, "query_id %s", query_id);

//             // ENVIO PAQUETE DE CONFIRMACION
//             t_paquete *paquete_create = crear_paquete(CREATE_RECIBO);
//             agregar_a_paquete(paquete_create, file_tag, strlen(file_tag) + 1);
//             // agregar_a_paquete(paquete_create, query_id, strlen(query_id) + 1);
//             enviar_paquete(paquete_create, socket);
//             eliminar_paquete(paquete_create);

//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_CREATE", socket, CREATE_RECIBO);
//             list_destroy_and_destroy_elements(partes_create, free);
//             break;
//         }
//         case ENVIO_TRUNCATE:
//         {
//             t_list *partes_truncate;
//             recibir_paquete(socket, &partes_truncate);
//             Operacion_Truncate(partes_truncate);
//             // char *query_id = list_get(partes_truncate, 3);
//             // t_paquete *paquete_create = crear_paquete(CREATE_RECIBO);
//             // agregar_a_paquete(paquete_create, query_id, strlen(query_id) + 1);
//             // enviar_paquete(paquete_create, socket);
//             // eliminar_paquete(paquete_create);
//             // RESPONDER CON EL MOCK OBJECT;
//             enviar_mensaje("OK_TRUNCATE", socket, TRUNCATE_RECIBO);
//             list_destroy_and_destroy_elements(partes_truncate, free);
//             break;
//         }
//         case ENVIO_WRITE:
//         {
//             t_list *partes_write;
//             recibir_paquete(socket, &partes_write);
//             // void Operacion_write(t_list * partes_write);
//             list_destroy_and_destroy_elements(partes_write, free);
//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_WRITE", socket, WRITE_RECIBO);
//             break;
//         }
//         case ENVIO_READ:
//         {
//             t_list *partes_read;
//             recibir_paquete(socket, &partes_read);

//             // NUEVO PARA READ
//             // Operacion_Read(partes_read);

//             list_destroy_and_destroy_elements(partes_read, free);
//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_READ", socket, READ_RECIBO);
//             break;
//         }
//         case ENVIO_TAG:
//         {
//             t_list *partes_tag;
//             recibir_paquete(socket, &partes_tag);
//             Operacion_TAG(partes_tag);
//             // RESPONDER CON EL MOCK OBJECT;
//             enviar_mensaje("OK_TAG", socket, TAG_RECIBO);
//             list_destroy_and_destroy_elements(partes_tag, free);
//             break;
//         }
//         case ENVIO_COMMIT:
//         {
//             t_list *partes_commit;
//             recibir_paquete(socket, &partes_commit);
//             Operacion_Commit(partes_commit);

//             list_destroy_and_destroy_elements(partes_commit, free);
//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_COMMIT", socket, COMMIT_RECIBO);
//             break;
//         }
//         case ENVIO_FLUSH:
//         {
//             t_list *partes_flush;
//             recibir_paquete(socket, &partes_flush);
//             // Operacion_Flush(partes_flush);

//             list_destroy_and_destroy_elements(partes_flush, free);
//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_FLUSH", socket, FLUSH_RECIBO);
//             break;
//         }
//         case PEDIR_BLOQUE_FALTANTE:
//         {
//             t_list *solicitud_pagina;
//             recibir_paquete(socket, &solicitud_pagina);

//             char *file_tag = list_get(solicitud_pagina, 0);
//             int *nro_pagina_ptr = (int *)list_get(solicitud_pagina, 1);
//             int nro_pagina = *nro_pagina_ptr;

//             // OBTIENE PARÁMETROS GLOBALES NECESARIOS:
//             char *punto_de_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
//             int block_size = config_get_int_value(config_super_block, "BLOCK_SIZE");
//             int cantidad_bloques = Cantidad_Bloques(); // Cantidad_Bloques() está definida en estructuras_basicas.c

//             log_info(logger, "[PAGE FAULT] Solicitud de bloque faltante: File:Tag %s, Bloque Nro %d", file_tag, nro_pagina);

//             // LLAMADA A LA FUNCIÓN REAL DEL FS
//             char *contenido_pagina = obtener_contenido_bloque_fs(file_tag, nro_pagina, block_size, logger, punto_de_montaje, cantidad_bloques);

//             if (contenido_pagina != NULL)
//             {
//                 // Envío de la Respuesta OK al Worker
//                 t_paquete *paquete_respuesta = crear_paquete(RESPUESTA_BLOQUE_FALTANTE_OK);

//                 // Protocolo de respuesta: [file_tag, nro_pagina, contenido binario]
//                 agregar_a_paquete(paquete_respuesta, file_tag, strlen(file_tag) + 1);
//                 agregar_a_paquete(paquete_respuesta, nro_pagina_ptr, sizeof(int));
//                 agregar_a_paquete(paquete_respuesta, contenido_pagina, block_size);
//                 log_debug(logger, "Envio read: %s", contenido_pagina);

//                 enviar_paquete(paquete_respuesta, socket);
//                 eliminar_paquete(paquete_respuesta);

//                 free(contenido_pagina);
//                 log_info(logger, "[PAGE FAULT] Contenido del bloque Nro %d enviado correctamente.", nro_pagina);
//             }
//             else
//             {
//                 log_error(logger, "[PAGE FAULT] ERROR: Fallo al obtener el contenido de la página. No se envía contenido.");
//                 // Si tienes un protocolo de ERROR, lo enviarías aquí. Si no, simplemente el Worker esperará un timeout o manejo de error.
//             }

//             list_destroy_and_destroy_elements(solicitud_pagina, free);
//             break;
//         }
//         case GUARDAR_FRAME:
//         {
//             // --- 1. RECEPCIÓN DE DATOS DEL WORKER ---
//             t_list *paquete_write;
//             recibir_paquete(socket, &paquete_write);

//             // Desempaquetar los datos recibidos (FILE, TAG, NRO_PAGINA_LOGICA, CONTENIDO)
//             char *file = list_get(paquete_write, 0);
//             char *tag = list_get(paquete_write, 1);
//             int *nro_pagina_logica_ptr = list_get(paquete_write, 2);
//             int nro_bloque_logica = *nro_pagina_logica_ptr;
//             char *contenido = list_get(paquete_write, 3);

//             // Obtener valores de configuración
//             char *punto_de_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
//             int tamanio_bloque = config_get_int_value(config_super_block, "BLOCK_SIZE");
//             int cantidad_bloques = Cantidad_Bloques();

//             // Rutas
//             char *path_metadata = path_metadata_logical(punto_de_montaje, file, tag);

//             // Variables de trabajo
//             int indice_bloque_fisico_actual;
//             int indice_bloque_fisico_final;
//             char *ruta_bloque_a_escribir;

//             // --- BLOQUEO CRÍTICO: Protección de Metadatos y Bitmap ---
//             pthread_mutex_lock(&mux_fs);

//             // --- 2. OBTENER EL BLOQUE FÍSICO ACTUALMENTE ASIGNADO ---
//             linea_bloques_t linea_info = Leer_Linea_Bloques(path_metadata);
//             if (linea_info.linea == NULL)
//             {
//                 // Error de lectura de metadata.
//                 pthread_mutex_unlock(&mux_fs);
//                 // ... Manejo de error y cleanup
//             }
//             t_list *bloques_fisicos_asignados = Obtener_Bloques_Desde_Linea(linea_info.linea);

//             // if (nro_bloque_logica >= list_size(bloques_fisicos_asignados)) //! ESTA MEDIO EXTRAÑO
//             // {
//             //     // Bloque lógico fuera de rango (error de Worker o Truncate incompleto)
//             //     pthread_mutex_unlock(&mux_fs);
//             //     // ... Manejo de error y cleanup
//             // }

//             long *nro_bloque_fisico_ptr = list_get(bloques_fisicos_asignados, nro_bloque_logica);
//             indice_bloque_fisico_actual = (int)*nro_bloque_fisico_ptr;

//             // t_list *bloques = Obtener_Bloques_Desde_Linea(linea_info.linea);
//             //     long *array_bloques = Lista_A_Array(bloques, &cantidad_bloques_logicos_array);

//             //     int i = 0;
//             // while (i < cantidad_bloques)
//             // {
//             //     char *path_fisico_bloque = path_physical_blocks(punto_de_montaje, i, cantidad_bloques);

//             //     // Iterar sobre los bloques lógicos del tag y buscar coincidencias
//             //     int j = 0;
//             //     int coincidencia_encontrada = 0;
//             //     while (j < cantidad_bloques_logicos_array && !coincidencia_encontrada)
//             //     {
//             //         char *path_logico_bloques = path_logical_block(punto_de_montaje, file, tag, j);

//             //         // Si el bloque lógico apunta al bloque físico
//             //         if (Apunta_A_Bloque_Fisico(path_logico_bloques, path_fisico_bloque))
//             //         {
//             //             // Liberar bloque físico y hacer unlink del bloque lógico
//             //             Liberar_Bloque_Fisico(path_logico_bloques, path_fisico_bloque, i);
//             //             coincidencia_encontrada = 1; // Evita iteraciones innecesarias
//             //         }

//             //         free(path_logico_bloques);
//             //         j++;
//             //     }
//             //     free(path_fisico_bloque);
//             //     i++;
//             // }

//             // Si dos procesos (o dos punteros, o dos estructuras) apuntan al mismo bloque compartido, ambos leen del mismo bloque.

//             // En el momento en que uno de los dos quiere escribir, se crea una copia privada del bloque para ese escritor.

//             // El otro proceso sigue apuntando al bloque original, sin verse afectado.
//             free(linea_info.linea);
//             list_destroy_and_destroy_elements(bloques_fisicos_asignados, free);

//             // --- 3. CONTAR REFERENCIAS (Decisión COW) ---
//             nlink_t referencias = contar_referencias_a_bloque(indice_bloque_fisico_actual, punto_de_montaje, cantidad_bloques);
//             log_warning(logger, "refs = %ju\n", (uintmax_t)referencias);
//             if (referencias > 1)
//             {
//                 // --- CASO 1: COPY-ON-WRITE NECESARIO (Bloque Compartido) ---
//                 log_warning(logger, "Bloque Físico %d compartido (%ju referencias). Aplicando Copy-on-Write.",
//                             indice_bloque_fisico_actual, (uintmax_t)referencias);

//                 // 4a. BUSCAR Y ASIGNAR NUEVO BLOQUE FÍSICO (Actualiza el Bitmap y persiste)
//                 indice_bloque_fisico_final = buscar_bloque_libre_y_marcar();

//                 if (indice_bloque_fisico_final == -1)
//                 {
//                     log_error(logger, "ERROR: No hay bloques físicos libres para COW.");
//                     pthread_mutex_unlock(&mux_fs);
//                     enviar_mensaje("GUARDAR_FRAME_ERROR: No hay espacio.", socket, RESPUESTA_GUARDAR_FRAME);
//                     list_destroy_and_destroy_elements(paquete_write, free);
//                     // free(path_metadata);
//                     break;
//                 }
//             }
//             else
//             {
//                 // --- CASO 2: ESCRITURA DIRECTA (Referencia Única) ---
//                 log_info(logger, "Bloque Físico %d con referencia única. Escribiendo directamente.", indice_bloque_fisico_actual);
//                 indice_bloque_fisico_final = indice_bloque_fisico_actual;
//             }

//             // 4b. ACTUALIZAR METADATOS DEL FILE:TAG (Apunta al nuevo bloque asignado)
//             actualizar_metadatos_bloque(path_metadata, nro_bloque_logica, indice_bloque_fisico_final);

//             // --- 5. ESCRIBIR EL CONTENIDO EN EL BLOQUE FÍSICO FINAL ---
//             ruta_bloque_a_escribir = path_physical_blocks(punto_de_montaje, indice_bloque_fisico_final, cantidad_bloques);

//             escribir_contenido_en_bloque(ruta_bloque_a_escribir, contenido, tamanio_bloque);
//             free(ruta_bloque_a_escribir);

//             pthread_mutex_unlock(&mux_fs); // Liberar el bloqueo

//             // --- 6. RESPONDER AL WORKER ---
//             enviar_mensaje("GUARDAR_FRAME_OK", socket, RESPUESTA_GUARDAR_FRAME_OK);

//             free(path_metadata);
//             list_destroy_and_destroy_elements(paquete_write, free);
//             break;
//         }
//         case ENVIO_DELETE:
//         {
//             t_list *partes_delete;
//             recibir_paquete(socket, &partes_delete);
//             Operacion_Delete(partes_delete);

//             list_destroy_and_destroy_elements(partes_delete, free);
//             // RESPONDER CON EL MOCK OBJECT;
//             // enviar_mensaje("OK_DELETE", socket, DELETE_RECIBO);
//             break;
//         }
//         case -1:
//         {
//             log_error(logger, "Error de comunicación con el cliente");
//             close(socket);
//             pthread_exit(NULL);
//             break;
//         }
//         default:
//             log_warning(logger, "operacion desconocida: %d", cod_op);
//         }
//     }
// }
// void esperar_clientes_de_storage(int socket_servidor) //* Espera a los Workers para crear el hilo para procesar la informacion.
// {
//     while (1)
//     {
//         log_debug(logger, "esperando clientes...");
//         int socket_cliente = esperar_cliente(socket_servidor, logger);
//         int *socket_ptr = malloc(sizeof(int));
//         *socket_ptr = socket_cliente;
//         pthread_t espacio_cliente;
//         pthread_create(&espacio_cliente, NULL, atencion_clientes_de_storage, (void *)socket_ptr);
//         pthread_detach(espacio_cliente);
//     }
// }