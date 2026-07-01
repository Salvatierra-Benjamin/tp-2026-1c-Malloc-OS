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
typedef enum
{
    LEER,
    ESCRIBIR
} t_operacion_memoria;

// Busca el Memory Stick que contiene la dirección física actual
t_memory_stick *encontrar_stick_por_direccion(t_list *sticks, uint32_t dir_fisica)
{
    for (int i = 0; i < list_size(sticks); i++)
    {
        t_memory_stick *stick = list_get(sticks, i);
        // Validar si la dirección cae dentro de la base y el límite del stick
        if (dir_fisica >= stick->base && dir_fisica < (stick->base + stick->tamanio))
        {
            return stick;
        }
    }
    return NULL;
}

void operar_en_memoria(uint32_t dir_fisica, void *buffer, uint32_t tamano, t_operacion_memoria op, t_list *memory_sticks)
{
    uint32_t bytes_restantes = tamano;
    uint32_t offset_buffer = 0;
    uint32_t dir_actual = dir_fisica;

    while (bytes_restantes > 0)
    {
        t_memory_stick *stick = encontrar_stick_por_direccion(memory_sticks, dir_actual);

        if (stick == NULL)
        {
            // Manejo de error: Dirección física no mapeada a ningún stick
            return;
        }

        // Calcular cuántos bytes podemos operar en el stick actual
        uint32_t bytes_disponibles_en_stick = (stick->base + stick->tamanio) - dir_actual;
        uint32_t bytes_a_operar = (bytes_restantes < bytes_disponibles_en_stick) ? bytes_restantes : bytes_disponibles_en_stick;

        // Acá deberías serializar y hacer el send() por socket al stick correspondiente
        if (op == LEER)
        {
            // TODO: Enviar petición de lectura a stick->socket por 'bytes_a_operar' en 'dir_actual'
            // TODO: Hacer recv() y guardar la respuesta en (buffer + offset_buffer)
        }
        else
        {
            // TODO: Enviar petición de escritura a stick->socket por 'bytes_a_operar' en 'dir_actual' enviando (buffer + offset_buffer)
            // TODO: Esperar la confirmación del stick (OK)
        }

        // Actualizamos los punteros y contadores para el próximo ciclo (por si saltamos a otro stick)
        bytes_restantes -= bytes_a_operar;
        offset_buffer += bytes_a_operar;
        dir_actual += bytes_a_operar;
    }
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
// “## PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.

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

// Variable global o inyectada por configuración

extern t_list *lista_memory_sticks;

void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char *param2)
{
    (void)param2;

    uint32_t dir_logica = contexto->registros.SI;                     // Se lee de SI
    uint32_t tamano_a_leer = obtener_tamano_registro(registro_datos); // Ej: EAX = 4, AX = 1

    t_mmu_result mmu = mmu_traducir(dir_logica, tamano_a_leer, contexto, TAM_MAX_SEGMENTO);

    if (mmu.seg_fault)
    {
        // TODO: Devolver el proceso al Kernel Scheduler con motivo SEG_FAULT
        return;
    }

    void *buffer = malloc(tamano_a_leer);
    operar_en_memoria(mmu.dir_fisica, buffer, tamano_a_leer, LEER, lista_memory_sticks);

    setear_valor_registro(contexto, registro_datos, buffer); // Asignamos los datos al registro
    free(buffer);
}

void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos)
{
    uint32_t dir_logica = contexto->registros.DI; // Se escribe en DI
    uint32_t tamano_a_escribir = obtener_tamano_registro(registro_datos);

    t_mmu_result mmu = mmu_traducir(dir_logica, tamano_a_escribir, contexto, TAM_MAX_SEGMENTO);

    if (mmu.seg_fault)
    {
        // TODO: Devolver el proceso al Kernel Scheduler con motivo SEG_FAULT
        return;
    }

    void *valor_a_escribir = leer_valor_registro(contexto, registro_datos);
    operar_en_memoria(mmu.dir_fisica, valor_a_escribir, tamano_a_escribir, ESCRIBIR, lista_memory_sticks);
}

void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *nombre_registro_tamano)
{
    uint32_t dir_logica_origen = contexto->registros.SI;
    uint32_t dir_logica_destino = contexto->registros.DI;

    // Obtenemos el valor guardado en el registro que indica cuántos bytes copiar
    uint32_t tamano = obtener_valor_registro_como_entero(contexto, nombre_registro_tamano);

    t_mmu_result mmu_origen = mmu_traducir(dir_logica_origen, tamano, contexto, TAM_MAX_SEGMENTO);
    if (mmu_origen.seg_fault)
    {
        // TODO: Devolver el proceso al Kernel Scheduler con motivo SEG_FAULT
        return;
    }

    t_mmu_result mmu_destino = mmu_traducir(dir_logica_destino, tamano, contexto, TAM_MAX_SEGMENTO);
    if (mmu_destino.seg_fault)
    {
        // TODO: Devolver el proceso al Kernel Scheduler con motivo SEG_FAULT
        return;
    }

    void *buffer = malloc(tamano);

    // Primero leemos consolidando los sticks de origen
    operar_en_memoria(mmu_origen.dir_fisica, buffer, tamano, LEER, lista_memory_sticks);

    // Luego escribimos consolidando los sticks de destino
    operar_en_memoria(mmu_destino.dir_fisica, buffer, tamano, ESCRIBIR, lista_memory_sticks);

    free(buffer);
}

// MOV_IN (Registro Datos): Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro SI y lo almacena en el Registro Datos.;
// MOV_OUT (Registro Datos): Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro DI.;
// COPY_MEM (Registro Tamaño): Toma la dirección apuntada por el registro SI y copia la cantidad de bytes indicadas en el registro tamaño a la posición de memoria apuntada por el registro DI.;

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

    flag_syscall = 1;
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

    flag_syscall = 1;
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

    flag_syscall = 1;
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

    flag_syscall = 1;
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

    flag_syscall = 1;
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

    flag_syscall = 1;
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
    // PROCESAR DIRECCION FISICA Y TAMAÑO. POR AHORA LE PASAMOS LA LOGICA PARA VER COMO SE ORGANIZA EL KM.
    t_paquete *paquete_stdout = crear_paquete(ENVIO_STDOUT);
    agregar_a_paquete(paquete_stdout, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_stdout, &direccion_logica, sizeof(uint32_t));
    agregar_a_paquete(paquete_stdout, &tamanio, sizeof(uint32_t));

    enviar_paquete(paquete_stdout, conexion_ks);

    eliminar_paquete(paquete_stdout);

    flag_syscall = 1;
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
    // PROCESAR DIRECCION FISICA Y TAMAÑO. POR AHORA LE PASAMOS LA LOGICA PARA VER COMO SE ORGANIZA EL KM.
    t_paquete *paquete_stdin = crear_paquete(ENVIO_STDIN);
    agregar_a_paquete(paquete_stdin, &(contexto->PID), sizeof(int));
    agregar_a_paquete(paquete_stdin, &direccion_logica, sizeof(uint32_t));
    agregar_a_paquete(paquete_stdin, &tamanio, sizeof(uint32_t));

    enviar_paquete(paquete_stdin, conexion_ks);

    eliminar_paquete(paquete_stdin);

    flag_syscall = 1;
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

    flag_syscall = 1;
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

    flag_syscall = 1;
}
