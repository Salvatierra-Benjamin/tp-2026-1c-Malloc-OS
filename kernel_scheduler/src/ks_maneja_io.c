#include "ks_maneja_io.h"

void *ks_maneja_io(void *arg) {
    int fd = *(int*)arg;
    free(arg);

    log_debug(ks_logger, "Entra en ks_maneja_io");
    // espera IO_ENVIA_IO
    // int op = recibir_operacion(fd);
    // if (op != IO_ENVIA_IO)
    // {
    //     close(fd);
    //     return;
    // }

    // // t_paquete *paquete = recibir_paquete(fd);
    // // paquete->buffer->offset = 0;

    // // char *path = read_net_string_from_pkg(paquete);
    // // uint32_t prio = 0;
    // // buffer_read(&prio, paquete->buffer, sizeof(uint32_t));
    // // eliminar_paquete(paquete);

    // // crear IO

    // // log_io_conectado(path); // faltan datos, pero despues se completa

    // // mantener conexion abierta: leer hasta desconexion
    // for (;;)
    // {
    //     int siguiente = recibir_operacion(fd);
    //     if (siguiente <= 0)
    //     {
    //         // Desconexion de IO -> cancelar su io
    //         // log_io_desconectado(path); // y finalizar segun estado.

    //         close(fd);
    //         return;
    //     }
    //     (void)recibir_paquete(fd); // descartar payload
    // }
}
