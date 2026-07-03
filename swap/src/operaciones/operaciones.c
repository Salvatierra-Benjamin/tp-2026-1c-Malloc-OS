#include "operaciones.h"
#include <string.h>

// Un bloque es válido si entra completo dentro de SWAP_FILE_SIZE.
bool bloque_valido(int bloque)
{
    int cantidad_bloques = swap_config.swap_file_size / swap_config.swap_block_size;
    return bloque >= 0 && bloque < cantidad_bloques;
}

int escribir_bloque(int bloque, void *datos)
{
    if (!bloque_valido(bloque))
    {
        log_error(swap_logger, "Escritura rechazada: bloque %d fuera de rango", bloque);
        return -1;
    }

    // 1. Calculamos el desplazamiento (offset) en bytes
    long offset = (long)bloque * swap_config.swap_block_size;

    // 2. Nos posicionamos en el archivo exactamente en el inicio del bloque
    fseek(archivo_swap, offset, SEEK_SET);

    // 3. Escribimos exactamente 1 bloque de tamaño swap_block_size
    if (fwrite(datos, swap_config.swap_block_size, 1, archivo_swap) != 1)
    {
        log_error(swap_logger, "Error al escribir el bloque %d", bloque);
        return -1;
    }

    // 4. Forzamos la escritura física en disco
    fflush(archivo_swap);

    return swap_config.swap_block_size;
}

int leer_bloque(int bloque, void *buffer)
{
    if (!bloque_valido(bloque))
    {
        log_error(swap_logger, "Lectura rechazada: bloque %d fuera de rango", bloque);
        return -1;
    }

    // 1. Calculamos el desplazamiento (offset) en bytes
    long offset = (long)bloque * swap_config.swap_block_size;

    // 2. Nos posicionamos en el archivo
    fseek(archivo_swap, offset, SEEK_SET);

    // 3. Leemos exactamente 1 bloque
    size_t leidos = fread(buffer, swap_config.swap_block_size, 1, archivo_swap);

    // 4. Verificamos que la lectura fue exitosa (debería leer 1 elemento del tamaño pedido)
    if (leidos == 1)
    {
        return swap_config.swap_block_size; // Retorna cantidad de bytes leídos
    }
    else
    {
        log_error(swap_logger, "Error al leer el bloque %d", bloque);
        return -1; // Retorna error
    }
}
