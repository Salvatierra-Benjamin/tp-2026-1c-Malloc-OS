#include "esperar_debug_client.h"
void esperar_debug_client()
{
    while (1)
    {
        log_debug(debug_server_logger, "Esperando debug_client...");
        int socket_cliente = esperar_cliente(socket_servidor_con_debug_client);
        int *socket_cliente_puntero = malloc(sizeof(int));
        *socket_cliente_puntero = socket_cliente;
        pthread_t hilo_atencion_debug_client;
        pthread_create(&hilo_atencion_debug_client, NULL, atencion_debug_client, (void *)socket_cliente_puntero);
        pthread_detach(hilo_atencion_debug_client);
    }
}

void *atencion_debug_client(void *socket_cliente_puntero)
{
    log_debug(debug_server_logger, "Se conecto un debug_client");
    int socket_cliente = *((int *)(socket_cliente_puntero));
    free(socket_cliente_puntero);

    while (1)
    {

        int cod_op = recibir_operacion(socket_cliente);

        if (cod_op < 0)
        {
            log_error(debug_server_logger, "cod op %d", cod_op);
            log_error(debug_server_logger, "debug_client desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case HANDSHAKE_DEBUG_CLIENT:
        {
            t_paquete *paquete_handshake_debug_server = recibir_paquete(socket_cliente);
            int debug_client_id;
            buffer_read(&debug_client_id, paquete_handshake_debug_server->buffer, sizeof(int));
            eliminar_paquete(paquete_handshake_debug_server);
            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Recibido el debug_client_id número: %d", debug_client_id);
            enviar_operacion(socket_cliente, HANDSHAKE_OK);

            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Enviando Mensaje para Alojar Proceso");
            t_paquete *paquete = crear_paquete(ALOJAR_PROCESO);
            PID_a_ejecutar = 120;
            agregar_a_paquete(paquete, &PID_a_ejecutar, sizeof(int));
            enviar_paquete(paquete, socket_cliente);
            eliminar_paquete(paquete);

            break;
        }
        case SOLICITUD_CONTEXTO_EJECUCION:
        {

            t_paquete *paquete_recibido = recibir_paquete(socket_cliente);
            int pid_recibido = -1; // SE PODRIA USAR PARA COMPARAR O BUSCAR EL PROCESO EN ESPECICO, ACÁ SOLO LO USO A MODO DE EJEMPLO
            buffer_read(&pid_recibido, paquete_recibido->buffer, sizeof(int));
            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Se Solicito el Contexto del PID: %d", pid_recibido);
            eliminar_paquete(paquete_recibido);

            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Armando Paquete con Contexto para mi Cliente");

            t_contexto_ejecucion contexto_actual;
            contexto_actual.PID = 120;

            // 1. Rellenar los registros planos
            contexto_actual.registros.PC = 1;
            contexto_actual.registros.AX = 2;
            contexto_actual.registros.BX = 3;
            contexto_actual.registros.CX = 4;
            contexto_actual.registros.DX = 5;
            contexto_actual.registros.EAX = 6;
            contexto_actual.registros.EBX = 7;
            contexto_actual.registros.ECX = 8;
            contexto_actual.registros.EDX = 9;
            contexto_actual.registros.SI = 10;
            contexto_actual.registros.DI = 11;

            // 2. Definir la cantidad de segmentos
            contexto_actual.tabla_segmentos.cantidad_segmentos = 12;

            // 3. Reservamos memoria para los 12 segmentos
            contexto_actual.tabla_segmentos.segmento = malloc(sizeof(segmento_t) * contexto_actual.tabla_segmentos.cantidad_segmentos);

            // 4. Rellenar datos de prueba en los segmentos
            contexto_actual.tabla_segmentos.segmento[0].dir_base = 13;
            contexto_actual.tabla_segmentos.segmento[0].tam_seg = 20;

            contexto_actual.tabla_segmentos.segmento[1].dir_base = 100;
            contexto_actual.tabla_segmentos.segmento[1].tam_seg = 512;

            // ==========================================
            //  SERIALIZACIÓN Y ENVÍO DEL CONTEXTO
            // ==========================================

            // 5. Creamos el paquete con el código de operación solicitado
            t_paquete *paquete = crear_paquete(CONTEXTO_EJECUCION);

            // 6. Agregamos los datos fijos (PID y la estructura completa de registros)
            agregar_a_paquete(paquete, &(contexto_actual.PID), sizeof(int));
            agregar_a_paquete(paquete, &(contexto_actual.registros), sizeof(t_registros));

            // 7. Agregamos la cantidad de segmentos (indispensable para que el receptor sepa cuánto medir)
            agregar_a_paquete(paquete, &(contexto_actual.tabla_segmentos.cantidad_segmentos), sizeof(uint32_t));

            // 8. Agregamos el contenido real del array dinámico pasándole el puntero directo
            size_t tamano_tabla = sizeof(segmento_t) * contexto_actual.tabla_segmentos.cantidad_segmentos;
            agregar_a_paquete(paquete, contexto_actual.tabla_segmentos.segmento, tamano_tabla);

            // 9. Enviamos el paquete serializado al cliente
            enviar_paquete(paquete, socket_cliente);

            // ==========================================
            //  LIMPIEZA DE MEMORIA (EVITAMOS MEMORY LEAKS)
            // ==========================================
            eliminar_paquete(paquete);                      // Liberamos el stream de red y el t_paquete
            free(contexto_actual.tabla_segmentos.segmento); // Liberamos la memoria del array que creamos arriba
            log_warning(debug_server_logger, "[RECEPTOR_debug_client] CONTEXTO ENVIADO CON EXITO!!!");
            break;
        }
        case SOLICITUD_INSTRUCCION:
        {
            t_paquete *paquete_recibido = recibir_paquete(socket_cliente);
            int pid_recibido = -1; // SE PODRIA USAR PARA COMPARAR O BUSCAR EL PROCESO EN ESPECICO, ACÁ SOLO LO USO A MODO DE EJEMPLO
            int pc_recibido = -1;  // SE PODRIA USAR PARA COMPARAR O BUSCAR EL PROCESO EN ESPECICO, ACÁ SOLO LO USO A MODO DE EJEMPLO
            buffer_read(&pid_recibido, paquete_recibido->buffer, sizeof(int));
            buffer_read(&pc_recibido, paquete_recibido->buffer, sizeof(int));
            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Se Solicito la Instruccion con PID: %d y el PC: %d", pid_recibido, pc_recibido);
            eliminar_paquete(paquete_recibido);

            log_debug(debug_server_logger, "[RECEPTOR_debug_client] Preparando paquete con Instruccion 120 para mi Cliente");
            // ==========================================
            //  1. ARMADO DE LA INSTRUCCIÓN DE PRUEBA
            // ==========================================
            t_instruccion instr_prueba;
            instr_prueba.comando = SET; // Usamos el enum directamente
            instr_prueba.PID = 120;

            // Simulamos la creación dinámica de los strings con malloc
            // (En el TP real esto seguro lo leas de un archivo, pero acá hardcodeamos para testear)

            // 1. HARDCODEAMOS LOS VALORES DE PRUEBA
            char *texto_param1 = "AX";
            char *texto_param2 = ""; // SIMULO QUE MANDO UN PARAMETRO CON STRING VACIO, ES DECIR, TAMAÑO = 1.
            // char *texto_param2 = NULL;  // SIMULO QUE MANDO UN PARAMETRO VACIO, ES DECIR, TAMAÑO = 0.
            // char *texto_param2 = "150"; // SIMULO UN PARAMETRO CUALQUIERA.

            // ASIGNACIÓN PARAMETRO 1 (Normal)
            instr_prueba.parametro_1 = malloc(strlen(texto_param1) + 1);
            strcpy(instr_prueba.parametro_1, texto_param1);

            // ASIGNACIÓN PARAMETRO 2 (Defensivo para el test)
            if (texto_param2 == NULL)
            {
                instr_prueba.parametro_2 = NULL;
            }
            else
            {
                instr_prueba.parametro_2 = malloc(strlen(texto_param2) + 1);
                strcpy(instr_prueba.parametro_2, texto_param2);
            }

            // 2. MEDIMOS LOS TAMÁÑOS USANDO 'instr_prueba' (Acá corregimos los nombres)
            // MODIFICACION PARA EVITAR POSIBLE SEGMENTATION FAULT: Si la instruccion no lleva segundo parametro y la estructura instruccion->parametro_2 apunte a NULL
            // al hacer strlen el programa crasheará.
            int parametro_1_size = (instr_prueba.parametro_1 != NULL) ? (strlen(instr_prueba.parametro_1) + 1) : 0;
            int parametro_2_size = (instr_prueba.parametro_2 != NULL) ? (strlen(instr_prueba.parametro_2) + 1) : 0;
            // Si el parámetro existe pero está vacío(""), strlen da 0, le suma 1, y te manda 1(viaja el \0). La CPU lo atrapa en len <= 1.
            // Si el parámetro directamente no existe(NULL), manda 0. La CPU lo atrapa en len <= 1.

            // ==========================================
            //  2. SERIALIZACIÓN (PATRÓN TLV)
            // ==========================================
            t_paquete *paquete = crear_paquete(ENVIO_INSTRUCCION);

            // A. Datos fijos (Comando y PID)
            agregar_a_paquete(paquete, &(instr_prueba.comando), sizeof(t_palabra_reservada));
            agregar_a_paquete(paquete, &(instr_prueba.PID), sizeof(int));

            // B. Parámetro 1 (Tamaño y Contenido)
            agregar_a_paquete(paquete, &parametro_1_size, sizeof(int));
            if (parametro_1_size > 0)
            {
                agregar_a_paquete(paquete, instr_prueba.parametro_1, parametro_1_size);
            }

            // C. Parámetro 2 (Tamaño y Contenido defensivo)
            agregar_a_paquete(paquete, &parametro_2_size, sizeof(int));
            if (parametro_2_size > 0)
            {
                agregar_a_paquete(paquete, instr_prueba.parametro_2, parametro_2_size);
            }

            // ==========================================
            //  3. ENVÍO Y LIMPIEZA
            // ==========================================
            enviar_paquete(paquete, socket_cliente);

            // Liberamos el paquete y los mallocs locales
            eliminar_paquete(paquete);
            free(instr_prueba.parametro_1);
            free(instr_prueba.parametro_2);
            log_warning(debug_server_logger, "[RECEPTOR_debug_client] INSTRUCCION ENVIADA CON EXITO!!!");
            break;
        }
        default:
        {
            log_warning(debug_server_logger, "[RECEPTOR_debug_client]Operacion Desconocida: %d", cod_op);
            break;
        }
        }
    }
}
