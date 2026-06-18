#include "operaciones.h"
#include <string.h>


void escribir_bloque(int bloque, void* datos) {
    // 1. Calculamos el desplazamiento (offset) en bytes
    long offset = bloque * swap_config.swap_block_size;

    // 2. Nos posicionamos en el archivo exactamente en el inicio del bloque
    fseek(archivo_swap, offset, SEEK_SET);

    // 3. Escribimos exactamente 1 bloque de tamaño swap_block_size
    fwrite(datos, swap_config.swap_block_size, 1, archivo_swap);

    // 4. (Opcional pero recomendado) Forzamos la escritura física en disco
    fflush(archivo_swap);
}

int leer_bloque(int bloque, void* buffer) {
    // 1. Calculamos el desplazamiento (offset) en bytes
    long offset = bloque * swap_config.swap_block_size;

    // 2. Nos posicionamos en el archivo
    fseek(archivo_swap, offset, SEEK_SET);

    // 3. Leemos exactamente 1 bloque
    size_t leidos = fread(buffer, swap_config.swap_block_size, 1, archivo_swap);

    // 4. Verificamos que la lectura fue exitosa (debería leer 1 elemento del tamaño pedido)
    if (leidos == 1) {
        return swap_config.swap_block_size; // Retorna cantidad de bytes leídos
    } else {
        log_error(swap_logger, "Error al leer el bloque %d", bloque);
        return -1; // Retorna error
    }
}