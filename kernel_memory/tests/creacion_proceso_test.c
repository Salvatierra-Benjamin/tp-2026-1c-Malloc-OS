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
}