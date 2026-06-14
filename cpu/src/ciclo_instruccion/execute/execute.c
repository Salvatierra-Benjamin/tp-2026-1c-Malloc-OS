#include "execute.h"

/*
===============================================
    FUNCIONES AUXILIARES
===============================================

Para obtener valores y su tamaño.
Devolver solo el puntero crudo del registro puede causar seg. fault ya que los int son 4 bytes.
*/

t_info_registro obtener_info_registro(t_contexto_ejecucion *contexto, char *nombre_registro)
{
    t_info_registro info = {NULL, 0};

    // Registros de 8 bits (1 byte)
    if (strcmp(nombre_registro, "AX") == 0)
    {
        info.puntero = &(contexto->registros.AX);
        info.tamanio = sizeof(uint8_t);
    }
    else if (strcmp(nombre_registro, "BX") == 0)
    {
        info.puntero = &(contexto->registros.BX);
        info.tamanio = sizeof(uint8_t);
    }
    else if (strcmp(nombre_registro, "CX") == 0)
    {
        info.puntero = &(contexto->registros.CX);
        info.tamanio = sizeof(uint8_t);
    }
    else if (strcmp(nombre_registro, "DX") == 0)
    {
        info.puntero = &(contexto->registros.DX);
        info.tamanio = sizeof(uint8_t);
    }

    // Registros de 32 bits (4 bytes)
    else if (strcmp(nombre_registro, "EAX") == 0)
    {
        info.puntero = &(contexto->registros.EAX);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "EBX") == 0)
    {
        info.puntero = &(contexto->registros.EBX);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "ECX") == 0)
    {
        info.puntero = &(contexto->registros.ECX);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "EDX") == 0)
    {
        info.puntero = &(contexto->registros.EDX);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "SI") == 0)
    {
        info.puntero = &(contexto->registros.SI);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "DI") == 0)
    {
        info.puntero = &(contexto->registros.DI);
        info.tamanio = sizeof(uint32_t);
    }
    else if (strcmp(nombre_registro, "PC") == 0)
    {
        info.puntero = &(contexto->registros.PC);
        info.tamanio = sizeof(uint32_t);
    }
    else
    {
        log_error(cpu_logger, "Registro desconocido: %s", nombre_registro);
    }

    return info;
}

uint32_t leer_registro(t_contexto_ejecucion *contexto, char *registro)
{
    t_info_registro info = obtener_info_registro(contexto, registro);

    if (info.puntero == NULL)
        return 0; // Error

    if (info.tamanio == sizeof(uint8_t))
    {
        return (uint32_t)(*(uint8_t *)info.puntero); // Leo 1 byte
    }
    else
    {
        return *(uint32_t *)info.puntero; // Leo 4 bytes
    }
}

void escribir_registro(t_contexto_ejecucion *contexto, char *registro, uint32_t valor)
{
    t_info_registro info = obtener_info_registro(contexto, registro);

    if (info.puntero == NULL)
        return; // Error

    if (info.tamanio == sizeof(uint8_t))
    {
        *(uint8_t *)info.puntero = (uint8_t)valor; // Escribo solo 1 byte (trunca si es mayor)
    }
    else
    {
        *(uint32_t *)info.puntero = valor; // Escribo 4 bytes
    }
}

int pasaje_de_char_a_int(char *palabra)
{
    char *endptr;
    // strtol siempre devuelve long, así que lo guardamos en un long temporal
    long num_long = strtol(palabra, &endptr, 10);

    // Lo casteamos al int que vos necesitás
    int num = (int)num_long;

    if (*endptr != '\0')
    {
        // Si entra acá, significa que no era un número puro (ej: "123abc")
        log_warning(cpu_logger, "El string tiene caracteres no numéricos: %s", endptr);
    }
    else
    {
        // Usamos %d porque ahora num es de tipo int
        log_info(cpu_logger, "Numero en String: %s", palabra);
        log_info(cpu_logger, "Numero en Int: %d", num);
    }

    return num;
}

/*
===============================================
    FIN FUNCIONES AUXILIARES
===============================================
*/

// =========================================================
// INSTRUCCIONES DE CPU PURA (Matematica y Control)
// =========================================================

void execute_NOOP(t_contexto_ejecucion *contexto, char *param1, char *param2)
{
    (void)param1;
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: NOOP", contexto->PID);
}

void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str)
{
    uint32_t valor = (uint32_t)pasaje_de_char_a_int(valor_str);

    escribir_registro(contexto, registro, valor);

    log_info(cpu_logger, "PID: %d - Ejecutando: SET - %s - %s", contexto->PID, registro, valor_str);
}

void execute_SUM(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen)
{
    uint32_t valor_destino = leer_registro(contexto, registro_destino);
    uint32_t valor_origen = leer_registro(contexto, registro_origen);

    uint32_t resultado = valor_destino + valor_origen;

    escribir_registro(contexto, registro_destino, resultado);

    log_info(cpu_logger, "PID: %d - Ejecutando: SUM - %s - %s", contexto->PID, registro_destino, registro_origen);
}

void execute_SUB(t_contexto_ejecucion *contexto, char *registro_destino, char *registro_origen)
{
    uint32_t valor_destino = leer_registro(contexto, registro_destino);
    uint32_t valor_origen = leer_registro(contexto, registro_origen);

    uint32_t resultado = valor_destino - valor_origen;

    escribir_registro(contexto, registro_destino, resultado);

    log_info(cpu_logger, "PID: %d - Ejecutando: SUB - %s - %s", contexto->PID, registro_destino, registro_origen);
}

void execute_JNZ(t_contexto_ejecucion *contexto, char *registro, char *instruccion_str)
{
    uint32_t valor_registro = leer_registro(contexto, registro);

    uint32_t nueva_instruccion = (uint32_t)pasaje_de_char_a_int(instruccion_str);

    if (valor_registro != 0)
    {
        contexto->registros.PC = nueva_instruccion;
    }
    log_info(cpu_logger, "PID: %d - Ejecutando: JNZ - %s - %s", contexto->PID, registro, instruccion_str);
}

// =========================================================
// INSTRUCCIONES DE MEMORIA (MMU) - TODO
// =========================================================

// COPY_MEM AX
// MOV_IN EDX
// MOV_OUT EDX

void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char *param2)
{
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MOV_IN - %s", contexto->PID, registro_datos);
    if (param2 == NULL)
    {
        log_error(cpu_logger, "PID: %d - MOV_IN sin direccion logica", contexto->PID);
        return;
    }

    t_info_registro info = obtener_info_registro(contexto, registro_datos);
    if (info.puntero == NULL)
    {
        return;
    }

    int direccion_logica = pasaje_de_char_a_int(param2);
    int dir_fisica;
    uint32_t bytes_validos;

    int mmu = MMU_TRADUCCION_FISICA(direccion_logica, info.tamanio, &dir_fisica, &bytes_validos);
    if (mmu == 0)
    {
        log_info(cpu_logger, "MMU llamada en MOV_IN");
    }
    if (bytes_validos != info.tamanio)
    {
        log_error(cpu_logger, "PID: %d - MOV_IN fallo en traduccion MMU para %s", contexto->PID, registro_datos);
    }
    // TODO: Implementar lógica de MMU (Traducción + Lectura de Memoria)
}

void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos, char *param2)
{
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MOV_OUT - %s", contexto->PID, registro_datos);

    if (param2 == NULL)
    {
        log_error(cpu_logger, "PID: %d - MOV_OUT sin direccion logica", contexto->PID);
        return;
    }

    t_info_registro info = obtener_info_registro(contexto, registro_datos);
    if (info.puntero == NULL)
    {
        return;
    }

    int direccion_logica = pasaje_de_char_a_int(param2);
    int dir_fisica;
    uint32_t bytes_validos;

    int mmu = MMU_TRADUCCION_FISICA(direccion_logica, info.tamanio, &dir_fisica, &bytes_validos);
    if (mmu == 0)
    {
        log_info(cpu_logger, "MMU llamada en MOV_OUT")
    }
    if (bytes_validos != info.tamanio)
    {
        log_error(cpu_logger, "PID: %d - MOV_OUT fallo en traduccion MMU para %s", contexto->PID, registro_datos);
    }

    uint8_t buffer[sizeof(uint32_t)] = {0};
    memcpy(buffer, info.puntero, info.tamanio); // TODO: Implementar lógica de MMU (Traducción + Escritura en Memoria)
}

// void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *tamano_str, char *param2)
// {
//     (void)param2;
//     log_info(cpu_logger, "PID: %d - Ejecutando: COPY_MEM - %s", contexto->PID, tamano_str);

//     int tamanio = pasaje_de_char_a_int(tamano_str);
//     if (tamanio <= 0)
//     {
//         log_error(cpu_logger, "PID: %d - COPY_MEM tamanio invalido: %s", contexto->PID, tamano_str);
//         return;
//     }

//     uint32_t origen_logico = leer_registro(contexto, "SI");
//     uint32_t destino_logico = leer_registro(contexto, "DI");
//     uint32_t restante = (uint32_t)tamanio;

//     void *buffer = malloc(restante);
//     if (buffer == NULL)
//     {
//         log_error(cpu_logger, "PID: %d - COPY_MEM fallo malloc", contexto->PID);
//         return;
//     }

//     while (restante > 0)
//     {
//         int dir_fisica_origen;
//         uint32_t bytes_origen;

//         int mmu = MMU_TRADUCCION_FISICA((int)origen_logico, restante, &dir_fisica_origen, &bytes_origen);
//         if (bytes_origen != info.tamanio)
//         {
//             log_error(cpu_logger, "PID: %d - COPY_MEM fallo traduccion origen SI", contexto->PID);
//             break;
//         }
//         if (bytes_validos != info.tamanio)
//         {
//             log_error(cpu_logger, "PID: %d - MOV_OUT fallo en traduccion MMU para %s", contexto->PID, registro_datos);
//         }

//         int dir_fisica_destino;
//         uint32_t bytes_destino;

//         int mmu2 = MMU_TRADUCCION_FISICA((int)destino_logico, restante, &dir_fisica_destino, &bytes_destino);
//         if (bytes_destino == 0)
//         {
//             log_error(cpu_logger, "PID: %d - COPY_MEM fallo traduccion destino DI", contexto->PID);
//             break;
//         }

//         uint32_t conjunto = bytes_origen < bytes_destino ? bytes_origen : bytes_destino;
//         if (conjunto > restante)
//         {
//             conjunto = restante;
//         }
//         origen_logico += conjunto;
//         destino_logico += conjunto;
//         restante -= conjunto;
//     }

//     free(buffer);
// }

// void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *nombre_registro_tamano, char *param2)
// {
//     (void)param2;
//     // 1. Obtenemos el tamaño a partir del REGISTRO
//     uint32_t tamanio = leer_registro(contexto, nombre_registro_tamano);

//     log_info(cpu_logger, "PID: %d - Ejecutando: COPY_MEM - Tamaño a copiar: %d", contexto->PID, tamanio);

//     if (tamanio <= 0)
//         return;

//     uint32_t origen_logico = leer_registro(contexto, "SI");
//     uint32_t destino_logico = leer_registro(contexto, "DI");
//     uint32_t restante = tamanio;

//     while (restante > 0)
//     {
//         int dir_fisica_origen, dir_fisica_destino;
//         uint32_t bytes_origen, bytes_destino;

//         // 2. Traducción origen
//         int mmu_origen = MMU_TRADUCCION_FISICA(origen_logico, restante, &dir_fisica_origen, &bytes_origen);
//         if (mmu_origen != 0)
//             return; // Si la MMU falla, ya envió el SEG_FAULT al Scheduler, solo cortamos la ejecución

//         // 3. Traducción destino
//         int mmu_destino = MMU_TRADUCCION_FISICA(destino_logico, restante, &dir_fisica_destino, &bytes_destino);
//         if (mmu_destino != 0)
//             return;

//         // 4. Determinar cuántos bytes podemos mover en este "salto" sin pasarnos de segmento en ninguno de los dos
//         uint32_t conjunto = (bytes_origen < bytes_destino) ? bytes_origen : bytes_destino;
//         if (conjunto > restante)
//             conjunto = restante;

//         // 5. Mover la data (ACÁ FALTA TU CÓDIGO DE RED)
//         void *buffer_temporal = malloc(conjunto);

//         // TODO: Enviar petición de lectura a dir_fisica_origen por 'conjunto' bytes
//         // TODO: Recibir la respuesta en buffer_temporal

//         // TODO: Enviar petición de escritura a dir_fisica_destino enviando buffer_temporal por 'conjunto' bytes
//         // TODO: Recibir confirmación (OK)

//         free(buffer_temporal);

//         // 6. Avanzar contadores
//         origen_logico += conjunto;
//         destino_logico += conjunto;
//         restante -= conjunto;
//     }
// }

// =========================================================
// SYSCALLS - TODO
// =========================================================

void execute_MUTEX_CREATE(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2)
{
    (void)param2;

    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_CREATE - %s", contexto->PID, nombre_mutex);
    t_paquete *paquete_mutex = crear_paquete(ENVIO_MUTEX_CREATE);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre + 1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);

    eliminar_paquete(paquete_mutex);
}

void execute_MUTEX_LOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2)
{
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_LOCK - %s", contexto->PID, nombre_mutex);

    t_paquete *paquete_mutex = crear_paquete(ENVIO_MUTEX_LOCK);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre + 1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);

    eliminar_paquete(paquete_mutex);
}

void execute_MUTEX_UNLOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char *param2)
{
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_UNLOCK - %s", contexto->PID, nombre_mutex);

    t_paquete *paquete_mutex = crear_paquete(ENVIO_MUTEX_UNLOCK);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre + 1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);

    eliminar_paquete(paquete_mutex);
}

void execute_MEM_ALLOC(t_contexto_ejecucion *contexto, char *tamano_str, char *registro_direccion)
{
    log_info(cpu_logger, "PID: %d - Ejecutando: MEM_ALLOC - %s bytes", contexto->PID, tamano_str);
    int tamanio = pasaje_de_char_a_int(tamano_str);
    if (tamanio <= 0)
    {
        log_error(cpu_logger, "PID: %d - MEM_ALLOC tamanio invalido: %s", contexto->PID, tamano_str);
        return;
    }


    t_paquete *paquete_mem = crear_paquete(ENVIO_MEM_ALLOC);
    agregar_a_paquete(paquete_mem, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_mem, &tamanio, sizeof(int));
    int tamanio_registro = strlen(registro_direccion);
    agregar_a_paquete(paquete_mem, &tamanio_registro, sizeof(int));
    agregar_a_paquete(paquete_mem, registro_direccion, tamanio_registro + 1);


    enviar_paquete(paquete_mem, conexion_ks);

    eliminar_paquete(paquete_mem);
}


void execute_MEM_FREE(t_contexto_ejecucion *contexto, char *registro_direccion, char *param2)
{
     (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MEM_FREE", contexto->PID);
   
    t_paquete *paquete_mem = crear_paquete(ENVIO_MEM_FREE);
    agregar_a_paquete(paquete_mem, &(contexto->PID), sizeof(int));
    int tamanio_registro = strlen(registro_direccion);
    agregar_a_paquete(paquete_mem, &tamanio_registro, sizeof(int));
    agregar_a_paquete(paquete_mem, registro_direccion, tamanio_registro + 1);


    enviar_paquete(paquete_mem, conexion_ks);

    eliminar_paquete(paquete_mem);


}

void execute_SLEEP(t_contexto_ejecucion *contexto, char *tiempo_str, char *param2)
{
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: SLEEP - %s", contexto->PID, tiempo_str);
   
    int tiempo = pasaje_de_char_a_int(tiempo_str);
    if (tiempo < 0)
    {
        log_error(cpu_logger, "PID: %d - SLEEP tiempo invalido: %s", contexto->PID, tiempo_str);
        return;
    }


    t_paquete *paquete_sleep = crear_paquete(ENVIO_SLEEP);
    agregar_a_paquete(paquete_sleep, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_sleep, &tiempo, sizeof(int));


    enviar_paquete(paquete_sleep, conexion_ks);

    eliminar_paquete(paquete_sleep);

}

void execute_STDOUT(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano)
{
    log_info(cpu_logger, "PID: %d - Ejecutando: STDOUT", contexto->PID);
   
    uint32_t direccion_logica = leer_registro(contexto, registro_direccion);
    uint32_t tamanio = leer_registro(contexto, registro_tamano);


    if (tamanio <= 0)
    {
        log_error(cpu_logger, "PID: %d - STDOUT tamanio invalido: %u", contexto->PID, tamanio);
        return;
    }
    t_paquete *paquete_stdout = crear_paquete(ENVIO_STDOUT);
    agregar_a_paquete(paquete_stdout, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_stdout, &direccion_logica, sizeof(uint32_t));
    agregar_a_paquete(paquete_stdout, &tamanio, sizeof(uint32_t));


    enviar_paquete(paquete_stdout, conexion_ks);

    eliminar_paquete(paquete_stdout);

}


void execute_STDIN(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano)
{
     log_info(cpu_logger, "PID: %d - Ejecutando: STDIN", contexto->PID);
   
    uint32_t direccion_logica = leer_registro(contexto, registro_direccion);
    uint32_t tamanio = leer_registro(contexto, registro_tamano);


    if (tamanio <= 0)
    {
        log_error(cpu_logger, "PID: %d - STDIN tamanio invalido: %u", contexto->PID, tamanio);
        return;
    }


    t_paquete *paquete_stdin = crear_paquete(ENVIO_STDIN);
    agregar_a_paquete(paquete_stdin, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_stdin, &direccion_logica, sizeof(uint32_t));
    agregar_a_paquete(paquete_stdin, &tamanio, sizeof(uint32_t));


    enviar_paquete(paquete_stdin, conexion_ks);

    eliminar_paquete(paquete_stdin);

}

void execute_INIT_PROC(t_contexto_ejecucion *contexto, char *path, char *prioridad_str)
{
     log_info(cpu_logger, "PID: %d - Ejecutando: INIT_PROC - %s", contexto->PID, path);
   
    int prioridad = pasaje_de_char_a_int(prioridad_str);
    if (prioridad < 0)
    {
        log_error(cpu_logger, "PID: %d - INIT_PROC prioridad invalida: %s", contexto->PID, prioridad_str);
        return;
    }


    t_paquete *paquete_init = crear_paquete(ENVIO_INIT_PROC);
    agregar_a_paquete(paquete_init, &(contexto->PID), sizeof(int));
    int tamanio_path = strlen(path);
    agregar_a_paquete(paquete_init, &tamanio_path, sizeof(int));
    agregar_a_paquete(paquete_init, path, tamanio_path + 1);
    agregar_a_paquete(paquete_init, &prioridad, sizeof(int));


    enviar_paquete(paquete_init, conexion_ks);

    eliminar_paquete(paquete_init);

}

void execute_EXIT(t_contexto_ejecucion *contexto, char *param1, char *param2)
{
    (void)param1;
    (void)param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: EXIT", contexto->PID);
   
    t_paquete *paquete_exit = crear_paquete(ENVIO_EXIT);
    agregar_a_paquete(paquete_exit, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_exit, conexion_ks);
    
    eliminar_paquete(paquete_exit);
}
