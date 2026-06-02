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
    if (strcmp(nombre_registro, "AX") == 0)      { info.puntero = &(contexto->registros.AX);  info.tamanio = sizeof(uint8_t); }
    else if (strcmp(nombre_registro, "BX") == 0) { info.puntero = &(contexto->registros.BX);  info.tamanio = sizeof(uint8_t); }
    else if (strcmp(nombre_registro, "CX") == 0) { info.puntero = &(contexto->registros.CX);  info.tamanio = sizeof(uint8_t); }
    else if (strcmp(nombre_registro, "DX") == 0) { info.puntero = &(contexto->registros.DX);  info.tamanio = sizeof(uint8_t); }
    
    // Registros de 32 bits (4 bytes)
    else if (strcmp(nombre_registro, "EAX") == 0) { info.puntero = &(contexto->registros.EAX); info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "EBX") == 0) { info.puntero = &(contexto->registros.EBX); info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "ECX") == 0) { info.puntero = &(contexto->registros.ECX); info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "EDX") == 0) { info.puntero = &(contexto->registros.EDX); info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "SI") == 0)  { info.puntero = &(contexto->registros.SI);  info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "DI") == 0)  { info.puntero = &(contexto->registros.DI);  info.tamanio = sizeof(uint32_t); }
    else if (strcmp(nombre_registro, "PC") == 0)  { info.puntero = &(contexto->registros.PC);  info.tamanio = sizeof(uint32_t); }
    else {
        log_error(cpu_logger, "Registro desconocido: %s", nombre_registro);
    }

    return info;
}


uint32_t leer_registro(t_contexto_ejecucion *contexto, char *registro)
{
    t_info_registro info = obtener_info_registro(contexto, registro);
    
    if (info.puntero == NULL) return 0; // Error

    if (info.tamanio == sizeof(uint8_t)) {
        return (uint32_t) (*(uint8_t*)info.puntero); // Leo 1 byte
    } else {
        return *(uint32_t*)info.puntero;             // Leo 4 bytes
    }
}

void escribir_registro(t_contexto_ejecucion *contexto, char *registro, uint32_t valor)
{
    t_info_registro info = obtener_info_registro(contexto, registro);
    
    if (info.puntero == NULL) return; // Error

    if (info.tamanio == sizeof(uint8_t)) {
        *(uint8_t*)info.puntero = (uint8_t)valor;    // Escribo solo 1 byte (trunca si es mayor)
    } else {
        *(uint32_t*)info.puntero = valor;            // Escribo 4 bytes
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

void execute_NOOP(t_contexto_ejecucion *contexto, char* param1, char* param2)
{
    (void) param1;
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: NOOP", contexto->PID);
}

void execute_SET(t_contexto_ejecucion *contexto, char *registro, char *valor_str)
{    
    uint32_t valor = (uint32_t) pasaje_de_char_a_int(valor_str);
    
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
    
    uint32_t nueva_instruccion = (uint32_t) pasaje_de_char_a_int(instruccion_str);
    
    if (valor_registro != 0) {
        contexto->registros.PC = nueva_instruccion;
    }    
    log_info(cpu_logger, "PID: %d - Ejecutando: JNZ - %s - %s", contexto->PID, registro, instruccion_str);

}


// =========================================================
// INSTRUCCIONES DE MEMORIA (MMU) - TODO
// =========================================================

void execute_MOV_IN(t_contexto_ejecucion *contexto, char *registro_datos, char* param2)
{
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MOV_IN - %s", contexto->PID, registro_datos);
    // TODO: Implementar lógica de MMU (Traducción + Lectura de Memoria)
}

void execute_MOV_OUT(t_contexto_ejecucion *contexto, char *registro_datos, char* param2)
{
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MOV_OUT - %s", contexto->PID, registro_datos);
    // TODO: Implementar lógica de MMU (Traducción + Escritura en Memoria)
}

void execute_COPY_MEM(t_contexto_ejecucion *contexto, char *tamano_str, char* param2)
{
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: COPY_MEM - %s", contexto->PID, tamano_str);
    // TODO: Implementar lógica de MMU (Doble traducción: origen en SI y destino en DI)
}


// =========================================================
// SYSCALLS - TODO
// =========================================================

void execute_MUTEX_CREATE(t_contexto_ejecucion *contexto, char *nombre_mutex, char* param2) {
    (void) param2;

    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_CREATE - %s", contexto->PID, nombre_mutex);
    t_paquete * paquete_mutex = crear_paquete(ENVIO_MUTEX_CREATE);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre +1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);
    
    eliminar_paquete(paquete_mutex);

}

void execute_MUTEX_LOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char* param2) {
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_LOCK - %s", contexto->PID, nombre_mutex);
    
    t_paquete * paquete_mutex = crear_paquete(ENVIO_MUTEX_LOCK);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre +1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);
    
    eliminar_paquete(paquete_mutex);
}

void execute_MUTEX_UNLOCK(t_contexto_ejecucion *contexto, char *nombre_mutex, char* param2) {
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MUTEX_UNLOCK - %s", contexto->PID, nombre_mutex);
    
    t_paquete * paquete_mutex = crear_paquete(ENVIO_MUTEX_UNLOCK);
    int tamanio_nombre = strlen(nombre_mutex);
    agregar_a_paquete(paquete_mutex, &tamanio_nombre, sizeof(int));
    agregar_a_paquete(paquete_mutex, nombre_mutex, tamanio_nombre +1);
    agregar_a_paquete(paquete_mutex, &(contexto->PID), sizeof(int));

    enviar_paquete(paquete_mutex, conexion_ks);
    
    eliminar_paquete(paquete_mutex);
}

void execute_MEM_ALLOC(t_contexto_ejecucion *contexto, char *tamano_str, char *registro_direccion) {
    log_info(cpu_logger, "PID: %d - Ejecutando: MEM_ALLOC - %s bytes", contexto->PID, tamano_str);
}

void execute_MEM_FREE(t_contexto_ejecucion *contexto, char *registro_direccion, char* param2) {
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: MEM_FREE", contexto->PID);
}

void execute_SLEEP(t_contexto_ejecucion *contexto, char *tiempo_str, char* param2) {
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: SLEEP - %s", contexto->PID, tiempo_str);
}

void execute_STDOUT(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano) {
    log_info(cpu_logger, "PID: %d - Ejecutando: STDOUT", contexto->PID);
}

void execute_STDIN(t_contexto_ejecucion *contexto, char *registro_direccion, char *registro_tamano) {
    log_info(cpu_logger, "PID: %d - Ejecutando: STDIN", contexto->PID);
}

void execute_INIT_PROC(t_contexto_ejecucion *contexto, char *path, char *prioridad_str) {
    log_info(cpu_logger, "PID: %d - Ejecutando: INIT_PROC - %s", contexto->PID, path);
}

void execute_EXIT(t_contexto_ejecucion *contexto, char* param1, char* param2) {
    (void) param1;
    (void) param2;
    log_info(cpu_logger, "PID: %d - Ejecutando: EXIT", contexto->PID);
}

