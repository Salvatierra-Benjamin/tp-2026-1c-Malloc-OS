#include "fetch.h"

void Iniciar_fetch()
{
    t_paquete *paquete_solicitud_instruccion = crear_paquete(SOLICITUD_INSTRUCCION);

    agregar_a_paquete(paquete_solicitud_instruccion, &contexto_ejecucion.PID, sizeof(int));
    agregar_a_paquete(paquete_solicitud_instruccion, &contexto_ejecucion.registros.PC, sizeof(int));
    enviar_paquete(paquete_solicitud_instruccion, conexion_km);
    eliminar_paquete(paquete_solicitud_instruccion);
}
void Ejecutar_fetch()
{
}