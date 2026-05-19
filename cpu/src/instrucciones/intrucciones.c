#include "intrucciones.h"
void NOOP()
{
    contexto_ejecucion.registros.PC += 1;
}
typedef struct
{
    void *ptr;
    size_t tam;
} t_registro_info;

static t_registro_info tabla_registros[] =
    {
        [AX] = {&contexto_ejecucion.registros.AX, sizeof(uint8_t)},
        [BX] = {&contexto_ejecucion.registros.BX, sizeof(uint8_t)},
        [CX] = {&contexto_ejecucion.registros.CX, sizeof(uint8_t)},
        [DX] = {&contexto_ejecucion.registros.DX, sizeof(uint8_t)},

        [EAX] = {&contexto_ejecucion.registros.EAX, sizeof(uint32_t)},
        [EBX] = {&contexto_ejecucion.registros.EBX, sizeof(uint32_t)},
        [ECX] = {&contexto_ejecucion.registros.ECX, sizeof(uint32_t)},
        [EDX] = {&contexto_ejecucion.registros.EDX, sizeof(uint32_t)},

        [SI] = {&contexto_ejecucion.registros.SI, sizeof(uint32_t)},
        [DI] = {&contexto_ejecucion.registros.DI, sizeof(uint32_t)},

        [PC] = {&contexto_ejecucion.registros.PC, sizeof(uint32_t)}};

void *obtener_reg_puntero(reg_id reg, size_t *tam)
{
    if (reg < 0 || reg >= (sizeof(tabla_registros) / sizeof(tabla_registros[0])))
    {
        *tam = 0;
        return NULL;
    }

    if (tabla_registros[reg].ptr == NULL)
    {
        *tam = 0;
        return NULL;
    }

    *tam = tabla_registros[reg].tam;
    return tabla_registros[reg].ptr;
}
// void *obtener_reg_puntero(reg_id reg, size_t *tam)
// {
//     switch (reg)
//     {
//     case AX:
//         *tam = sizeof(uint8_t);
//         return &contexto_ejecucion.registros.AX;
//     case BX:
//         *tam = sizeof(uint8_t);
//         return &contexto_ejecucion.registros.BX;
//     case CX:
//         *tam = sizeof(uint8_t);
//         return &contexto_ejecucion.registros.CX;
//     case DX:
//         *tam = sizeof(uint8_t);
//         return &contexto_ejecucion.registros.DX;

//     case EAX:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.EAX;
//     case EBX:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.EBX;
//     case ECX:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.ECX;
//     case EDX:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.EDX;

//     case SI:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.SI;
//     case DI:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.DI;
//     case PC:
//         *tam = sizeof(uint32_t);
//         return &contexto_ejecucion.registros.PC;
//     default:
//         *tam = 0;
//         return NULL;
//     }
// }
void SET(reg_id reg, uint32_t valor)
{
    size_t tam;
    void *ptr = obtener_reg_puntero(reg, &tam);

    if (!ptr)
        return;

    if (tam == sizeof(uint8_t))
    {
        *(uint8_t *)ptr = (uint8_t)valor; // truncamiento
    }
    else
    {
        *(uint32_t *)ptr = valor;
    }
}
uint32_t leer_reg(reg_id reg)
{
    size_t tam;
    void *ptr = obtener_reg_puntero(reg, &tam);
    if (!ptr)
    {
        log_debug(cpu_logger, "Error en la funcion leer_reg");
        exit(EXIT_FAILURE);
    }
    if (tam == sizeof(uint8_t))
    {
        (uint8_t)ptr;
    }
    else
    {
        (uint32_t)ptr;
    }
}
void escribir_reg(reg_id reg, uint32_t valor)
{
    size_t tam;
    void *ptr = obtener_reg_puntero(reg, &tam);
    if (!ptr)
    {
        log_debug(cpu_logger, "Error en la funcion escribir_reg");
        exit(EXIT_FAILURE);
    }
    if (tam == sizeof(uint8_t))
    {
        (uint8_t)ptr = (uint8_t)valor;
    }
    else
    {
        (uint32_t)ptr = valor;
    }
}

void leer_memoria(uint32_t dir_fisica, void *buffer_destino, uint32_t tamanio)
{
    // =========================================================
    // 1. ARMAR LA PETICIÓN
    // =========================================================
    // Usá el op_code que hayan definido en su protocolos.h
    t_paquete *paquete_peticion = crear_paquete(LEER_MEMORIA_STICK);

    // Le decimos a partir de dónde leer
    agregar_a_paquete(paquete_peticion, &dir_fisica, sizeof(uint32_t));

    // Le decimos cuántos bytes queremos que nos devuelva
    agregar_a_paquete(paquete_peticion, &tamanio, sizeof(uint32_t));

    // =========================================================
    // 2. ENVIAR POR RED
    // =========================================================
    enviar_paquete(paquete_peticion, conexion_ms);
    eliminar_paquete(paquete_peticion); // Siempre liberar la memoria del paquete enviado

    // =========================================================
    // 3. RECIBIR LA RESPUESTA (Queda bloqueado esperando)
    // =========================================================
    // Ojo: dependiendo de cómo sea tu red, quizás primero hacés un recv del OP_CODE
    // int op_code_respuesta = recibir_operacion(conexion_ms;

    t_paquete *paquete_respuesta = recibir_paquete(conexion_ms);

    // Verificamos que todo haya salido bien del otro lado
    if (paquete_respuesta->codigo_operacion == RESPUESTA_OK_LEER_MEMORIA)
    {
        // =========================================================
        // 4. PEGAR LOS DATOS EN EL BUFFER DE LA CPU
        // =========================================================
        // Usamos tu función buffer_read para sacar los datos serializados
        // y pegarlos directamente en el puntero que nos pasaron por parámetro.
        buffer_read(buffer_destino, paquete_respuesta->buffer, tamanio);
    }
    else
    {
        // Manejo de error si el Memory Stick falló o se desconectó
        log_error(cpu_logger, "Error: El Memory Stick no devolvió los datos esperados.");
    }

    // =========================================================
    // 5. LIMPIEZA
    // =========================================================
    eliminar_paquete(paquete_respuesta);
}

void MOV_IN(reg_id reg)
{
    size_t tam;
    // 1. Obtenemos el puntero directo al registro
    void *ptr = obtener_reg_puntero(reg, &tam);

    // 2. Origen lógico
    uint32_t dir_logica = contexto_ejecucion.registros.SI;

    uint32_t restante = tam;
    uint32_t actual = dir_logica;

    // 3. Buffer temporal para unificar todo
    uint8_t *buffer = malloc(tam);
    uint32_t offset = 0;

    // 4. Bucle para lidiar con fragmentación entre Memory Sticks
    while (restante > 0)
    {
        uint32_t dir_fisica;
        uint32_t bytes_validos;

        // OJO: Le pasamos 'restante', no 'tam'
        int res = MMU_TRADUCCION_FISICA(actual, restante, &dir_fisica, &bytes_validos);

        if (res < 0) // Si la MMU detecta SEG_FAULT
        {
            log_debug(cpu_logger, "SEG_FAULT en MOV_IN");
            free(buffer);
            // Acá además deberías devolver el PID al Scheduler por SegFault
            return;
        }

        // 5. Leer directo de la red hacia el offset de nuestro buffer
        leer_memoria(dir_fisica, buffer + offset, bytes_validos);

        // 6. Actualizar contadores
        offset += bytes_validos;
        restante -= bytes_validos;
        actual += bytes_validos;
    }

    // 7. Pegar todo el buffer armado directo en el registro original
    memcpy(ptr, buffer, tam);

    // 8. Limpiar
    free(buffer);
}

// TODO leer_memoria(dir_fisica, buffer + offset, bytes_validos);

void MOV_OUT()
{
}

void SUM(reg_id dest, reg_id orig)
{
    size_t tam_dest, tam_orig;

    void *ptr_dest = obtener_reg_puntero(dest, &tam_dest);
    void *ptr_orig = obtener_reg_puntero(orig, &tam_orig);

    if (!ptr_dest || !ptr_orig)
        return;

    // DESTINO 8 bits
    if (tam_dest == sizeof(uint8_t))
    {
        uint8_t *d = (uint8_t *)ptr_dest;

        if (tam_orig == sizeof(uint8_t))
            *d += *(uint8_t *)ptr_orig;
        else
            *d += (uint8_t)(*(uint32_t *)ptr_orig); // truncamiento
    }
    // DESTINO 32 bits
    else
    {
        uint32_t *d = (uint32_t *)ptr_dest;

        if (tam_orig == sizeof(uint8_t))
            *d += *(uint8_t *)ptr_orig;
        else
            *d += *(uint32_t *)ptr_orig;
    }
}

void SUB(reg_id orig, reg_id dest)
{
    size_t tam_dest, tam_orig;

    void *ptr_dest = obtener_reg_puntero(dest, &tam_dest);
    void *ptr_orig = obtener_reg_puntero(orig, &tam_orig);

    if (!ptr_dest || !ptr_orig)
        return;

    // DESTINO 8 bits
    if (tam_dest == sizeof(uint8_t))
    {
        uint8_t *d = (uint8_t *)ptr_dest;

        if (tam_orig == sizeof(uint8_t))
            *d -= *(uint8_t *)ptr_orig;
        else
            *d -= (uint8_t)(*(uint32_t *)ptr_orig); // truncamiento
    }
    // DESTINO 32 bits
    else
    {
        uint32_t *d = (uint32_t *)ptr_dest;

        if (tam_orig == sizeof(uint8_t))
            *d -= *(uint8_t *)ptr_orig;
        else
            *d -= *(uint32_t *)ptr_orig;
    }
}
void JNZ(reg_id registro, uint32_t instruccion)
{
    size_t tam;
    void *ptr = obtener_reg_puntero(registro, &tam);

    if (!ptr)
    {
        log_debug(cpu_logger, "instruccion incorrecta en jnz");
        return;
    }

    int es_cero = 0;

    if (tam == sizeof(uint8_t))
    {
        es_cero = (*(uint8_t *)ptr == 0);
    }
    else
    {
        es_cero = (*(uint32_t *)ptr == 0);
    }

    if (!es_cero)
    {
        contexto_ejecucion.registros.PC = instruccion;
    }
}
// TODO COPY_MEM
// void COPY_MEM(reg_id reg)
// {
//     uint32_t dir = contexto_ejecucion.registros.SI;

//     COPY_MEM(Registro Tamaño) : Toma la dirección apuntada por el registro SI y copia la cantidad de bytes
//                                     indicadas en el registro tamaño a la posición de memoria apuntada por el registro DI.
// }