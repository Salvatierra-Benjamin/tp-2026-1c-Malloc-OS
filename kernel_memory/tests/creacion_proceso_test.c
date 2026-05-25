#include <stdio.h>
#include <recursos_globales/recursos_globales.h>
#include <manejador_ks/manejador_ks.h>
#include <cspecs/cspec.h>

context(creacion_procesos_tests)
{
    describe("dividir_archivo_por_lineas")
    {
        before
        {
            logger = log_create("km.log", "LOGGER - KM", 0, LOG_LEVEL_ERROR);
        } end

        after
        {
            log_destroy(logger);
            logger = NULL;
        } end

        it("devuelve NULL si el archivo no existe")
        {
            t_list *resultado = dividir_archivo_por_lineas("/ruta/inexistente.txt");
            should_ptr(resultado) be null;
        } end

        it("devuelve una lista vacia si el archivo esta vacio")
        {
            t_list *resultado = dividir_archivo_por_lineas("/home/utnso/plug-n-pray-pruebas/test_vacio.pcp");
            should_ptr(resultado) not be null;
            should_int(list_size(resultado)) be equal to(0);
            list_destroy(resultado);
        } end

        it("lee correctamente las instrucciones de un archivo")
        {
            t_list *resultado = dividir_archivo_por_lineas("/home/utnso/plug-n-pray-pruebas/test_instrucciones.pcp");
            should_int(list_size(resultado)) be equal to(3);
            should_string((char *)list_get(resultado, 0)) be equal to("SET AX 5");
            should_string((char *)list_get(resultado, 1)) be equal to("SET BX 3");
            should_string((char *)list_get(resultado, 2)) be equal to("EXIT");

            list_destroy_and_destroy_elements(resultado, free);
        } end

        it("lee correctamente aunque la ultima linea no tenga salto de linea") {
            t_list *resultado = dividir_archivo_por_lineas("/home/utnso/plug-n-pray-pruebas/test_instrucciones_salto_linea.pcp");

            should_ptr(resultado) not be null;
            should_int(list_size(resultado)) be equal to(2);
            should_string((char *)list_get(resultado, 0)) be equal to("SET AX 5");
            should_string((char *)list_get(resultado, 1)) be equal to("EXIT");

            list_destroy_and_destroy_elements(resultado, free);
        } end
    } end
    
    describe("string a palabra reservada")
    {
        it("reconoce NOOP")
        {
            should_int(string_a_palabra_reservada("NOOP")) be equal to(NOOP);
        } end
        it("reconoce SET") 
        {
            should_int(string_a_palabra_reservada("SET")) be equal to(SET);
        } end

        it("reconoce MUTEX_CREATE (con underscore)") 
        {
            should_int(string_a_palabra_reservada("MUTEX_CREATE")) be equal to(MUTEX_CREATE);
        } end

        it("reconoce EXIT (último del enum)") 
        {
            should_int(string_a_palabra_reservada("EXIT")) be equal to(EXIT);
        } end

        it("retorna -1 ante un string desconocido") 
        {
            should_int(string_a_palabra_reservada("BANANA")) be equal to(-1);
        } end

        it("es case-sensitive: 'noop' no es válido") 
        {
            should_int(string_a_palabra_reservada("noop")) be equal to(-1);
        } end
    } end

    describe("palabra_reservada_a_string") {

        it("convierte NOOP")
        {
            should_string(palabra_reservada_a_string(NOOP)) be equal to("NOOP");
        } end

        it("convierte MEM_ALLOC")
        {
            should_string(palabra_reservada_a_string(MEM_ALLOC)) be equal to("MEM_ALLOC");
        } end

        it("convierte EXIT")
        {
            should_string(palabra_reservada_a_string(EXIT)) be equal to("EXIT");
        } end

        it("retorna NULL para un valor fuera del enum")
        {
            should_ptr(palabra_reservada_a_string(999)) be null;
        } end

        it("es inverso de string_a_palabra_reservada")
        {
            char *original = "JNZ";
            t_palabra_reservada cmd = string_a_palabra_reservada(original);
            should_string(palabra_reservada_a_string(cmd)) be equal to(original);
        } end

    } end

    describe("linea_a_instruccion")
    {

        t_instruccion *inst = NULL;

        after
        {
            if (inst != NULL)
            {
                inst = NULL;
            }
        } end

        it("parsea NOOP sin parámetros")
        {
            inst = linea_a_instruccion("NOOP");
            should_int(inst->comando) be equal to(NOOP);
            should_ptr(inst->parametro_1) be null;
            should_ptr(inst->parametro_2) be null;
        } end

        it("parsea EXIT sin parámetros")
        {
            inst = linea_a_instruccion("EXIT");
            should_int(inst->comando) be equal to(EXIT);
            should_ptr(inst->parametro_1) be null;
            should_ptr(inst->parametro_2) be null;
        } end

        it("parsea SLEEP con un parámetro numérico")
        {
            inst = linea_a_instruccion("SLEEP 25000");
            should_int(inst->comando) be equal to(SLEEP);
            should_string(inst->parametro_1) be equal to("25000");
            should_ptr(inst->parametro_2) be null;
        } end

        it("parsea MUTEX_LOCK con nombre como parámetro")
        {
            inst = linea_a_instruccion("MUTEX_LOCK MUTEX_1");
            should_int(inst->comando) be equal to(MUTEX_LOCK);
            should_string(inst->parametro_1) be equal to("MUTEX_1");
            should_ptr(inst->parametro_2) be null;
        } end

        it("parsea SET con registro y valor")
        {
            inst = linea_a_instruccion("SET AX 6");
            should_int(inst->comando) be equal to(SET);
            should_string(inst->parametro_1) be equal to("AX");
            should_string(inst->parametro_2) be equal to("6");
        } end

        it("parsea SUM con dos registros")
        {
            inst = linea_a_instruccion("SUM AX BX");
            should_int(inst->comando) be equal to(SUM);
            should_string(inst->parametro_1) be equal to("AX");
            should_string(inst->parametro_2) be equal to("BX");
        } end

        it("parsea MEM_ALLOC con dos numéricos")
        {
            inst = linea_a_instruccion("MEM_ALLOC 0 64");
            should_int(inst->comando) be equal to(MEM_ALLOC);
            should_string(inst->parametro_1) be equal to("0");
            should_string(inst->parametro_2) be equal to("64");
        } end

        it("parsea INIT_PROC con archivo y prioridad")
        {
            inst = linea_a_instruccion("INIT_PROC proceso1 3");
            should_int(inst->comando) be equal to(INIT_PROC);
            should_string(inst->parametro_1) be equal to("proceso1");
            should_string(inst->parametro_2) be equal to("3");
        } end

        it("duplica los parámetros (no aliasing al input)")
        {
            char linea[] = "SET AX 6";
            inst = linea_a_instruccion(linea);
            // Si parametro_1 apuntara a un offset de 'linea', esto rompería todo
            should_ptr(inst->parametro_1) not be equal to(linea + 4);
        } end

    } end

    describe("convertir_lineas_en_instrucciones")
    {

        t_list *lineas = NULL;
        t_list *instrucciones = NULL;

        before
        {
            lineas = list_create();
        } end

        after
        {
            if (lineas != NULL)
            {
                list_destroy_and_destroy_elements(lineas, free);
                lineas = NULL;
            }
            if (instrucciones != NULL)
            {
                list_destroy_and_destroy_elements(instrucciones, free);
                instrucciones = NULL;
            }
        } end

        it("convierte una lista vacía en una lista vacía")
        {
            instrucciones = convertir_lineas_en_instrucciones(lineas);
            should_int(list_size(instrucciones)) be equal to(0);
        } end

        it("convierte una única línea")
        {
            list_add(lineas, string_duplicate("NOOP"));
            instrucciones = convertir_lineas_en_instrucciones(lineas);

            should_int(list_size(instrucciones)) be equal to(1);
            t_instruccion *inst = list_get(instrucciones, 0);
            should_int(inst->comando) be equal to(NOOP);
        } end

        it("preserva el orden de las instrucciones") 
        {
            list_add(lineas, string_duplicate("SET AX 6"));
            list_add(lineas, string_duplicate("SUM AX BX"));
            list_add(lineas, string_duplicate("EXIT"));

            instrucciones = convertir_lineas_en_instrucciones(lineas);

            should_int(list_size(instrucciones)) be equal to(3);

            t_instruccion *primera = list_get(instrucciones, 0);
            should_int(primera->comando) be equal to(SET);
            should_string(primera->parametro_1) be equal to("AX");
            should_string(primera->parametro_2) be equal to("6");

            t_instruccion *segunda = list_get(instrucciones, 1);
            should_int(segunda->comando) be equal to(SUM);

            t_instruccion *tercera = list_get(instrucciones, 2);
            should_int(tercera->comando) be equal to(EXIT);
            should_ptr(tercera->parametro_1) be null;
        } end

        it("no modifica la lista de líneas original")
        {
            list_add(lineas, string_duplicate("NOOP"));
            list_add(lineas, string_duplicate("EXIT"));

            instrucciones = convertir_lineas_en_instrucciones(lineas);

            should_int(list_size(lineas)) be equal to(2);
            should_string((char*)list_get(lineas, 0)) be equal to("NOOP");
        } end

    } end
}