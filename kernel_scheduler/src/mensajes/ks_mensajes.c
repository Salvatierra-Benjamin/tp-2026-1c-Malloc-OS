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
	buffer_read (&len1, contenido->buffer, sizeof (int)) ; // LEEMOS CUANTO MIDE
	char* nombre_mutex = malloc(len1); // PEDIMOS EL ESPACIO JUSTO
	buffer_read (&nombre_mutex, contenido->buffer, len1); // COPIAMOS EL CONTENIDO DEL BUFFER
*/

// ! -------------------------------- KS RTA GRAL DE OK CONEXIÓN ------------------------------------------------------------------------------------------------
void enviar_ok_conexion(fd) {
	enviar_operacion(fd, KS_OK_CONEXION);
}

// ! -------------------------------- KS -> CPU ------------------------------------------------------------------------------------------------
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

void enviar_mem_alloc(int cpu_fd, int id_segmento, int tamanio, int pid) {
	t_paquete* paquete_alloc = crear_paquete(MEM_ALLOC);
	agregar_a_paquete(paquete_alloc, &pid, sizeof(int));
	agregar_a_paquete(paquete_alloc, &id_segmento, sizeof(int));
	agregar_a_paquete(paquete_alloc, &tamanio, sizeof(int));
	enviar_paquete(paquete_alloc, cpu_fd);
	eliminar_paquete(paquete_alloc);
}

void enviar_mem_free(int cpu_fd, int id_segmento, int pid) {
	t_paquete* paquete_free = crear_paquete(MEM_FREE);
	agregar_a_paquete(paquete_free, &pid, sizeof(int));
	agregar_a_paquete(paquete_free, &id_segmento, sizeof(int));
	enviar_paquete(paquete_free, cpu_fd);
	eliminar_paquete(paquete_free);
}

// ! -------------------------------- KS -> IO ------------------------------------------------------------------------------------------------
void enviar_sleep_io(int io_fd, int tiempo_ms, int pid) {
	t_paquete* paquete_sleep = crear_paquete(SLEEP_IO);
	agregar_a_paquete(paquete_sleep, &tiempo_ms, sizeof(int));
	agregar_a_paquete(paquete_sleep, &pid, sizeof(int));
	enviar_paquete(paquete_sleep, io_fd);
	eliminar_paquete(paquete_sleep);
}

void enviar_stdin_io(int io_fd, int tamanio, int pid) {
	t_paquete* paquete_stdin = crear_paquete(STDIN_IO);
	agregar_a_paquete(paquete_stdin, &tamanio, sizeof(int));
	agregar_a_paquete(paquete_stdin, &pid, sizeof(int));
	enviar_paquete(paquete_stdin, io_fd);
	eliminar_paquete(paquete_stdin);
}

void enviar_stdout_io(int io_fd, char* cadena_de_bytes, int len, int pid) {
	t_paquete* paquete_stdout = crear_paquete(STDOUT_IO);
	agregar_a_paquete(paquete_stdout, &len, sizeof(int));
	agregar_a_paquete(paquete_stdout, cadena_de_bytes, len);
	agregar_a_paquete(paquete_stdout, &pid, sizeof(int));
	enviar_paquete(paquete_stdout, io_fd);
	eliminar_paquete(paquete_stdout);
}

// ! -------------------------------- KS -> KM ------------------------------------------------------------------------------------------------
void enviar_bytes_stdin(int pid, int len, char* dir_logica, int tamanio) {
	t_paquete* paquete_stdin = crear_paquete(STDOUT_SOLICITUD_LECTURA);
	agregar_a_paquete(paquete_stdin, &len, sizeof(int));
	agregar_a_paquete(paquete_stdin, dir_logica, len);
	agregar_a_paquete(paquete_stdin, &tamanio, sizeof(int));
	agregar_a_paquete(paquete_stdin, &pid, sizeof(int));
	enviar_paquete(paquete_stdin, socket_km);
	eliminar_paquete(paquete_stdin);
}

void enviar_aviso_suspension_km(int pid) {
	t_paquete* paquete_suspension = crear_paquete(SUSPENCION_PROCESO);
	agregar_a_paquete(paquete_suspension, &pid, sizeof(int));
	enviar_paquete(paquete_suspension, socket_km);
	eliminar_paquete(paquete_suspension);
}

void enviar_lista_desuspension_km(t_list* lista_a_desuspender) {
	t_paquete* paquete_dessuspencion_lista = crear_paquete(SOLICITUD_DESSUSPENCION_LISTA);

    int cantidad = list_size(lista_a_desuspender);
    agregar_a_paquete(paquete_dessuspencion_lista, &cantidad, sizeof(int));

    for (int i = 0; i < cantidad; i++) {
        t_pcb* pcb = list_get(lista_a_desuspender, i);
        agregar_a_paquete(paquete_dessuspencion_lista, &(pcb->pid), sizeof(int));
        agregar_a_paquete(paquete_dessuspencion_lista, &(pcb->prioridad), sizeof(int));
    }

    enviar_paquete(paquete_dessuspencion_lista, socket_km);
    eliminar_paquete(paquete_dessuspencion_lista);
}