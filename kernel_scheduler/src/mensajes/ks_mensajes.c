#include "ks_mensajes.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"

/* // ! EJEMPLO DE USO
    t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
    agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
    enviar_paquete(paquete_solicitud_contexto, conexion_km);
    eliminar_paquete(paquete_solicitud_contexto);

*/

void enviar_interrupcion_compactacion_cpu(int cpu_fd, int pid) {
	t_paquete* paquete_interrupcion = crear_paquete(SOLICITUD_DESALOJO_COMPACTACION_MEMORIA);
	agregar_a_paquete(paquete_interrupcion, &pid, sizeof(int));
	enviar_paquete(paquete_interrupcion, cpu_fd);
	eliminar_paquete(paquete_interrupcion);
}

void enviar_interrupcion_cpu(int cpu_fd, int pid, char* motivo) {
	t_paquete* paquete_interrupcion = crear_paquete(INTERRUPCION);
	agregar_a_paquete(paquete_interrupcion, &pid, sizeof(int));
	agregar_a_paquete(paquete_interrupcion, motivo, strlen(motivo) + 1);
	enviar_paquete(paquete_interrupcion, cpu_fd);
	eliminar_paquete(paquete_interrupcion);
}