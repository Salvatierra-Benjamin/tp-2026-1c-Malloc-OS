#include <main_io.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "ARGUMENTOS INSUFICIENTES\n");
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    char *tipo_io = argv[2];
    char *tipo_io_mayusculas = normalizar(tipo_io);

    if (strcmp(tipo_io_mayusculas, "STDIN") == 0)
    {
        imprimir_tipo_de_io(tipo_io_mayusculas);
        // TODO logica de STDIN
    }
    else if (strcmp(tipo_io_mayusculas, "STDOUT") == 0)
    {
        imprimir_tipo_de_io(tipo_io_mayusculas);
        // TODO logica de STDOUT
    }
    else if (strcmp(tipo_io_mayusculas, "SLEEP") == 0)
    {
        imprimir_tipo_de_io(tipo_io_mayusculas);
        // TODO logica de SLEEP
    }
    else
    {
        printf("Tipo de IO incorrecto");
        return -1;
    }

    io_cargar_config(path);
    inicializar_logger();
    imprimir_valores_config();

    int conexion_ks = crear_conexion(io_config.ip_ks, io_config.puerto_ks);
    if (conexion_ks == -1)
    {
        log_info(io_logger, "ERROR AL CREAR CONEXION IO->KS.\n");
        io_liberar_config();
        io_liberar_log();
        return -1;
    }

    enviar_operacion(conexion_ks, HANDSHAKE_IO);

    int recepcion_operacion = recibir_operacion(conexion_ks);
    if (recepcion_operacion == IO_OK_CONEXION)
    {
        log_debug(io_logger, "Se recibio respuesta OK de KS");
    }
    else
    {
        log_info(io_logger, "Error al recibir respuesta OK de KS");
        return -1;
    }

    log_debug(io_logger, "IO TERMINO.\n");

    free(tipo_io_mayusculas);
    io_liberar_config();
    io_liberar_log();

    return 0;
}
