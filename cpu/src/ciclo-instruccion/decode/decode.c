#include "decode.h"

void Decode_Instruccion(t_instruccion nueva_instruccion)
{
    // /* info: La etapa decode interpreta la instruccion y evalua traduccion de memoria. */
    // bool requiere_traduccion = false;

    // /* info: Log minimo y obligatorio de ejecucion de instruccion. */
    // log_info(cpu_logger, "## PID: %d - Ejecutando: %d - %s %s (139)",
    //          nueva_instruccion.PID,
    //          nueva_instruccion.comando,
    //          nueva_instruccion.parametro_1,
    //          nueva_instruccion.parametro_2);

    // switch (nueva_instruccion.comando)
    // {
    // case MOV_IN:
    // case MOV_OUT:
    // case COPY_MEM:
    // case STDIN:
    // case STDOUT:
    //     /* info: Estas instrucciones interactuan con memoria y usan mmu. */
    //     requiere_traduccion = true;
    //     break;

    // case NOOP:
    // case SET:
    // case SUM:
    // case SUB:
    // case JNZ:
    // case MUTEX_CREATE:
    // case MUTEX_LOCK:
    // case MUTEX_UNLOCK:
    // case MEM_ALLOC:
    // case MEM_FREE:
    // case SLEEP:
    // case INIT_PROC:
    // case EXIT:
    //     /* info: Operaciones de cpu o syscalls que no requieren traduccion en decode. */
    //     requiere_traduccion = false;
    //     break;

    // default:
    //     /* info: Manejo de instruccion desconocida o error. */
    //     log_error(cpu_logger, "Instruccion no reconocida.");
    //     return;
    // }

    // if (requiere_traduccion)
    // {
    //     /* info: Logica para interactuar con la mmu simulada antes de ejecutar. */
    //     // DIreccion Lógica:
    //     // INICIO contexto_ejecucion.registros.SI
    //     // FIN contexto_ejecucion.registros.DI

    //     uint32_t dir_fisica;
    //     MMU_TRADUCCION_FISICA(nueva_instruccion);
    // }

    // /* info: Una vez decodificada y con direcciones listas se avanza a execute. */
    // Execute_Instruccion(nueva_instruccion);
}
// #include "decode.h"
// #include <stdlib.h> // Para atoi

void Decode_Instruccion(t_instruccion nueva_instruccion)
{
    bool requiere_traduccion = false;

    // ... (tu log_info) ...

    switch (nueva_instruccion.comando)
    {
        // ... otros cases ...

    case SET:
    {
        requiere_traduccion = false;

        /* ACA ESTA EL DECODE FÍSICO:
           Dejamos de hablar en "strings" y pasamos a punteros y enteros */

        void *puntero_reg = obtener_registro(nueva_instruccion.parametro_1);
        int tamanio_reg = obtener_tamanio_registro(nueva_instruccion.parametro_1);
        uint32_t valor_num = (uint32_t)atoi(nueva_instruccion.parametro_2);

        // Una vez decodificado, le pasamos la posta a la etapa Execute
        ejecutar_SET(puntero_reg, tamanio_reg, valor_num);
        break;
    }

    case MOV_IN:
    {
        requiere_traduccion = true;

        // Decode del registro destino
        void *puntero_reg = obtener_registro(nueva_instruccion.parametro_1);
        int tamanio_reg = obtener_tamanio_registro(nueva_instruccion.parametro_1);

        // Decode de la dirección (La MMU actúa acá)
        uint32_t dir_logica = contexto.registros.SI;
        uint32_t dir_fisica = mmu_traducir_direccion(dir_logica); // Tu función de MMU

        // Pasamos al Execute todo resuelto
        ejecutar_MOV_IN(puntero_reg, tamanio_reg, dir_fisica);
        break;
    }

        // ...
    }
}
