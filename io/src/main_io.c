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
    imprimir_valores_config_io();

    //! PONER FUNCIONES ARRIBA DE LA  "creacion_conexiones_swap()" Ó EN EL RECEPTOR EN SU RESPECTIVO CASE (RECOMENDADO CREAR NUEVOS CASES PARA EL SWITCH) PORQUE ESTA UTILIZANDO UN HILO TIPO JOIN PARA QUE EL PROGRAMA NO SE CIERRER.
    //! NO USAR DETACH!!!! SALVO QUE HAYA UN BUCLE EN EL MAIN QUE SIRVA COMO ANCLAJE DEL PROGRAMA PARA QUE NO SE CIERRE!!!!
    creacion_conexiones_io();

    log_debug(io_logger, "IO TERMINO.\n");

    free(tipo_io_mayusculas);
    io_liberar_config();
    io_liberar_log();

    return 0;
}
