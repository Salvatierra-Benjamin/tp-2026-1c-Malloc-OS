#include "receptor_ks.h"
void receptor_ks()
{
    log_debug(io_logger, "[RECEPTOR_KS_IO] Conectando con KS...");
    pthread_t hilo_atencion_ks;
    pthread_create(&hilo_atencion_ks, NULL, atencion_ks, NULL);
    pthread_join(hilo_atencion_ks, NULL);
    // Si usás pthread_join: El hilo principal se queda "frizado" en esa línea. No podría avanzar a la siguiente función (como esperar_cpus).
    // Si usás pthread_detach: El hilo principal lanza el receptor y sigue de largo (OJO!!! PUEDE LLEGAR AL FIN DEL PROGRAMA!!!). Es la opción correcta si querés que el módulo haga más de una cosa a la vez (como ser cliente de ks y servidor de CPU al mismo tiempo).
}
char *funcion_stdin(int cant_bytes)
{
    char *cadena = malloc(cant_bytes); // Asigna memoria dinámica
    if (cadena == NULL)
    {
        // Manejo de error si malloc falla
        log_error(io_logger, "ERROR AL CREAR MALLOC STDIN");
        liberar_todo();
        exit(EXIT_FAILURE);
    }
    int index = 0;
    char caracter;
    printf("Ingrese la cadena a imprimir: ");
    while ((caracter = getchar()) != '\n' && index < cant_bytes)
    {
        cadena[index++] = caracter;
    }
    if (index < cant_bytes)
    {

        for (int i = index; i < cant_bytes; i++)
        {
            cadena[i] = '\0'; // Rellenar el resto de la cadena con caracteres nulos
        }
    }
    return cadena;
}

void *atencion_ks(void *arg)
{
    (void)arg;
    log_debug(io_logger, "[RECEPTOR_KS_IO] CONECTADO CON KS");
    while (1)
    {

        int cod_op = recibir_operacion(conexion_ks);

        if (cod_op < 0)
        {
            log_error(io_logger, "cod op %d", cod_op);
            log_error(io_logger, "KS desconectado o error");
            liberar_todo();
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KS_OK_CONEXION:
        {
            log_debug(io_logger, "[RECEPTOR_KS_IO] EL HANSHAKE CON KS SE HA REALIZADO CORRECTAMENTE!!!");
            log_info(io_logger, "## Conectado a Kernel Scheduler");
            t_paquete *paquete = crear_paquete(ENVIAR_DATOS_IO);
            int longitud_tipo_io = strlen(tipo_io_mayusculas);
            agregar_a_paquete(paquete, tipo_io_mayusculas, longitud_tipo_io);
            enviar_paquete(paquete, conexion_ks);
            eliminar_paquete(paquete);

            break;
        }
        case STDIN_IO:
        {
            if (strcmp(tipo_io_mayusculas, "STDIN") == 0)
            {

                t_paquete *paquete = recibir_paquete(conexion_ks);

                int pid;
                int cant_bytes;
                // KS debe enviar [cod_op, pid, cant_bytes]
                buffer_read(&pid, paquete->buffer, sizeof(int));
                buffer_read(&cant_bytes, paquete->buffer, sizeof(int));
                eliminar_paquete(paquete);

                log_info(io_logger, "## PID: %d - Inicio de IO", pid);
                // Inicio de IO: “## PID: <PID> - Inicio de IO”
                log_info(io_logger, "## PID: %d - Ingrese %d caracteres:", pid, cant_bytes);
                // Solo para STDIN: “## PID: <PID> - Ingrese <CANTIDAD A LEER> caracteres:”

                char *leido = funcion_stdin(cant_bytes);

                t_paquete *paquete_leido = crear_paquete(OK_STDIN);
                agregar_a_paquete(paquete_leido, leido, cant_bytes);

                enviar_paquete(paquete_leido, conexion_ks);

                log_info(io_logger, "## PID: %d - Fin de IO", pid);
                // Fin de IO : “##PID : <PID> - Fin de IO”

                free(leido);
                eliminar_paquete(paquete_leido);
            }
            else
            {
                log_error(io_logger, "IO y Operacion recibida es incorrecto");
            }
            break;
        }
        case STDOUT_IO:
        {
            if (strcmp(tipo_io_mayusculas, "STDOUT") == 0)
            {

                t_paquete *paquete = recibir_paquete(conexion_ks);

                int pid;
                int longitud_a_imprimir;
                // KS debe enviar [cod_op, pid, longitud_a_imprimir, cadena_a_imprimir]

                buffer_read(&pid, paquete->buffer, sizeof(int));
                buffer_read(&longitud_a_imprimir, paquete->buffer, sizeof(int));

                char *cadena_a_imprimir = malloc(longitud_a_imprimir + 1);
                buffer_read(cadena_a_imprimir, paquete->buffer, longitud_a_imprimir);
                cadena_a_imprimir[longitud_a_imprimir] = '\0'; // Agregado para que printf o log imprima hasta encontrar el \0

                log_info(io_logger, "## PID: %d - Inicio de IO", pid);
                // Inicio de IO: “## PID: <PID> - Inicio de IO”
                // OK_STDOUT: // OK_STDOUT_RESPUESTA_BYTES (PID)

                printf("Cadena recibida de KS: %s\n", cadena_a_imprimir);

                t_paquete *paquete_stdout = crear_paquete(OK_STDOUT);
                agregar_a_paquete(paquete_stdout, &pid, sizeof(int));

                enviar_paquete(paquete_stdout, conexion_ks);

                log_info(io_logger, "## PID: %d - %s ", pid, cadena_a_imprimir);
                // Solo para STDOUT: “## PID: <PID> - <CONTENIDO A IMPRIMIR>”

                log_info(io_logger, "## PID: %d - Fin de IO", pid);
                // Fin de IO : “##PID : <PID> - Fin de IO”

                eliminar_paquete(paquete);
                free(cadena_a_imprimir);
            }
            else
            {
                log_error(io_logger, "IO y Operacion recibida es incorrecto");
            }
            break;
        }
        case SLEEP_IO:
        {
            if (strcmp(tipo_io_mayusculas, "SLEEP") == 0)
            {

                t_paquete *paquete = recibir_paquete(conexion_ks);

                int pid;
                int tiempo_espera;
                buffer_read(&pid, paquete->buffer, sizeof(int));
                buffer_read(&tiempo_espera, paquete->buffer, sizeof(int));
                eliminar_paquete(paquete);

                log_info(io_logger, "## PID: %d - Inicio de IO", pid);
                // Inicio de IO: “## PID: <PID> - Inicio de IO”

                int tiempo_en_micro = tiempo_espera * 1000;
                usleep(tiempo_en_micro);

                t_paquete *paquete_sleep = crear_paquete(OK_SLEEP);
                agregar_a_paquete(paquete_sleep, &pid, sizeof(int));

                enviar_paquete(paquete_sleep, conexion_ks);

                log_info(io_logger, "## PID: %d - Haciendo sleep por %d milisegundos.", pid, tiempo_espera);
                // Solo para SLEEP: “## PID: <PID> - Haciendo sleep por <TIEMPO> milisegundos.”

                log_info(io_logger, "## PID: %d - Fin de IO", pid);
                // Fin de IO : “##PID : <PID> - Fin de IO”
            }
            else
            {
                log_error(io_logger, "IO y Operacion recibida es incorrecto");
            }
            break;
        }
        default:
        {
            log_warning(io_logger, "[RECEPTOR_KS_IO] operacion desconocida : %d", cod_op);
            break;
        }
        }
    }
}
