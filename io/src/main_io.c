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

    io_cargar_config(path);
    inicializar_logger();

    imprimir_valores_config_io();
    inicializar_semaforos_coordinadores();

    // !PONER FUNCIONES ARRIBA DE LA  "creacion_conexiones_swap()" Ó EN EL RECEPTOR EN SU RESPECTIVO CASE (RECOMENDADO CREAR NUEVOS CASES PARA EL SWITCH)
    // !PORQUE ESTA UTILIZANDO UN HILO TIPO JOIN PARA QUE EL PROGRAMA NO SE CIERRER.
    // !NO USAR DETACH!!!! SALVO QUE HAYA UN BUCLE EN EL MAIN QUE SIRVA COMO ANCLAJE DEL PROGRAMA PARA QUE NO SE CIERRE!!!!
    creacion_conexiones_io();

    log_debug(io_logger, "IO TERMINO.\n");

    free(tipo_io_mayusculas);
    liberar_todo();
    return 0;
}
