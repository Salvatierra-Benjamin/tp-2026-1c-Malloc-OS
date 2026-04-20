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
        return NULL;
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

void *atencion_ks()
{
    log_debug(io_logger, "[RECEPTOR_KS_IO] CONECTADO CON KS");
    while (1)
    {
        int cod_op = recibir_operacion(conexion_ks);

        if (cod_op < 0)
        {
            log_error(io_logger, "cod op %d", cod_op);
            log_error(io_logger, "KS desconectado o error");
            exit(EXIT_FAILURE);
        }
        switch (cod_op)
        {
        case KM_OK_CONEXION:
        {
            log_debug(io_logger, "[RECEPTOR_KS_IO] EL HANSHAKE CON KS SE HA REALIZADO CORRECTAMENTE!!!");
            t_list *paquete;
            recibir_paquete_equipo_cpu(conexion_ks, &paquete);
            int *pid = list_get(paquete, 0);
            log_info(io_logger, "## PID: %d - Inicio de IO", *pid);

            break;
        }
        case STDIN_IO:
        {
            if (strcmp(tipo_io_mayusculas, "STDIN") == 0)
            {
                t_list *paquete;
                recibir_paquete_equipo_cpu(conexion_ks, &paquete);
                int *cant_bytes = list_get(paquete, 0);
                int *pid = list_get(paquete, 1);
                char *leido = funcion_stdin(*cant_bytes);
                t_paquete *paquete_leido = crear_paquete(OK_STDIN);
                agregar_a_paquete(paquete_leido, leido, sizeof(leido));
                list_destroy_and_destroy_elements(paquete, free);
                free(leido);
                log_info(io_logger, "## PID: %d - Ingrese %s caracteres:", *pid, leido);
                // Solo para STDIN: “## PID: <PID> - Ingrese <CANTIDAD A LEER> caracteres:”
            }
        }
        case STDOUT_IO:
        {
            if (strcmp(tipo_io_mayusculas, "STOUT") == 0)
            {
                t_list *paquete;
                recibir_paquete_equipo_cpu(conexion_ks, &paquete);
                char *cadena_a_imprimir = list_get(paquete, 0);
                int *pid = list_get(paquete, 1);
                printf("Cadena recibida de KS: %s\n", cadena_a_imprimir);
                list_destroy_and_destroy_elements(paquete, free);
                enviar_operacion(conexion_ks, OK_STDOUT);
                log_info(io_logger, "## PID: %d - %s ", *pid, cadena_a_imprimir);
                // Solo para STDOUT: “## PID: <PID> - <CONTENIDO A IMPRIMIR>”
            }
        }
        case SLEEP_IO:
        {
            if (strcmp(tipo_io_mayusculas, "SLEEP") == 0)
            {

                t_list *paquete;
                recibir_paquete_equipo_cpu(conexion_ks, &paquete);
                int *tiempo_espera = list_get(paquete, 0);
                int *pid = list_get(paquete, 1);
                int tiempo_en_micro = *tiempo_espera * 1000;
                usleep(tiempo_en_micro);
                enviar_operacion(conexion_ks, OK_SLEEP);
                list_destroy_and_destroy_elements(paquete, free);
                log_info(io_logger, "## PID: %d - Haciendo sleep por %d milisegundos.", *pid, *tiempo_espera);
                // Solo para SLEEP: “## PID: <PID> - Haciendo sleep por <TIEMPO> milisegundos.”
            }
        }
        case -1:
        {
            log_error(io_logger, "Error de comunicación con la KS");
            io_liberar_config();
            io_liberar_log();
            exit(EXIT_FAILURE);
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
