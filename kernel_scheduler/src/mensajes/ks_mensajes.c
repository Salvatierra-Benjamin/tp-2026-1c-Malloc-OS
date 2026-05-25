#include "ks_mensajes.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"

/* // ! EJEMPLO DE USO
	// ! ENVIOS
    t_paquete *paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO_EJECUCION);
    // enviar int
	agregar_a_paquete(paquete_solicitud_contexto, &PID_a_ejecutar, sizeof(int));
	// enviar string
	int parametro_1_size = strlen (instruccion->parametro_1)+1;
	agregar_a_paquete (p_ins, &parametro_1_size, sizeof (int));
	agregar_a_paquete (p_ins, instruccion-parametro_1, parametro_1_size);

    enviar_paquete(paquete_solicitud_contexto, conexion_km);
    eliminar_paquete(paquete_solicitud_contexto);

	// ! RECEPCIONES
	int len1;
	buffer_read (&len1, paquete->buffer, sizeof (int)) ; // LEEMOS CUANTO MIDE
	nueva instruccion->parametro 1 = malloc(len1); // PEDIMOS EL ESPACIO JUSTO
	buffer_read (nueva_instruccion->parametro_1, paquete-›buffer, len1); // COPIAMOS EL CONTENIDO
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