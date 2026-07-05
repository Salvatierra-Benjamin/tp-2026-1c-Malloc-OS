#include "ks_maneja_km.h"

#include "../main/ks_main.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"
#include "../mensajes/ks_mensajes.h"
#include "../planificadores/ks_planificador_corto.h"
#include "../planificadores/ks_planificador_mediano.h"

void* ks_conexion_km(void* _arg) {
	(void)_arg;

	socket_km = crear_conexion(ks_config.ip_km, ks_config.puerto_km);

	if (socket_km < 0) {
		log_error(ks_logger, "No pude conectar a Kernel Memory %s:%s", ks_config.ip_km, ks_config.puerto_km);
		return NULL;
	}

	log_debug(ks_logger, "Conectado a Kernel Memory en %s:%s", ks_config.ip_km, ks_config.puerto_km);

	// HANDSHAKE
	enviar_operacion(socket_km, HANDSHAKE_KS);

	int op = recibir_operacion(socket_km);
	if (op <= 0) {
		log_error(ks_logger, "Kernel Memory cerró durante handshake");
		close(socket_km);
		return NULL;
	}

	// KM responde HANDSHAKE_OK genérico (manejador_global.c), no KM_OK_CONEXION
	if (op != HANDSHAKE_OK) {
		log_error(ks_logger, "Handshake invalido. Esperaba HANDSHAKE_OK (%d), llegó %d", HANDSHAKE_OK, op);
		close(socket_km);
		return NULL;
	}

	log_conexion_km(); // Loguear conexión a Kernel Memory
	// log_debug(ks_logger, "Handshake con Kernel Memory OK");

	// proceso inicial: se solicita recién acá porque necesita la conexión con KM establecida.
	// El pcb se crea cuando KM responda OK_CREACION_PROCESO.
	int pid_inicial = asignar_pid();
	registrar_creacion_pendiente(pid_inicial, ks_config.path_proceso_inicial, 0);
	enviar_solicitud_creacion_proceso_km(pid_inicial, ks_config.path_proceso_inicial, 0);

	// Loop de comunicación con KM
	while (1) {
		int cod_op = recibir_operacion(socket_km);

		if (cod_op <= 0) {
			log_error(ks_logger, "Kernel Memory se desconectó");
			close(socket_km);
			break;
		}

		switch (cod_op) {
		case DETECCION_CORRUPCION_MEMORIA: {
			matar_procesos_ready();
			liberar_listas_matando_procesos_bsod();
			liberar_mutex();
			destruir_semaforos();
			return NULL;
			// break;
		}

		case AUMENTO_DE_MEMORIA: { // AUMENTO_DE_MEMORIA (TAMAÑO)
			t_paquete* contenido = recibir_paquete(socket_km);

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			// ? Hacer algo con el tamanio? O solo es para loguear?

			sem_post(&sem_memoria_disponible);

			eliminar_paquete(contenido);
			break;
		}

			// case COMPACTACION: { // ? es lo mismo que DESALOJO_POR_COMPACTACION? O es para cuando KM detecta corrupción de memoria y decide compactar? O es para cuando KS detecta corrupción de memoria y decide matar procesos y compactar? No se si es necesario diferenciarlo, pero lo dejo por las dudas.
			//     log_inicio_compactacion();
			//     sem_post(&sem_memoria_disponible);

			// 	break;
			// }

			// case: // ! DETECCION_CORRUPCION_MEMORIA ---→ COMPACTACION
			// {

			// 	break;
			// }

		case DESALOJO_POR_COMPACTACION: {
			log_inicio_compactacion();

			pthread_mutex_lock(&mutex_compactacion);
			compactacion_en_progreso = true;
			pthread_mutex_unlock(&mutex_compactacion);

			ks_empieza_a_desalojar_por_compactacion();

			break;
		}

		case FIN_COMPACTACION: {
			log_fin_compactacion();
			pthread_mutex_lock(&mutex_compactacion);
			compactacion_en_progreso = false;
			pthread_mutex_unlock(&mutex_compactacion);

			sem_post(&sem_memoria_disponible);
			sem_post(&sem_ready);
			break;
		}

		case STDOUT_RESTUESTA_LECTURA: { // KM -> KS: [pid][tamanio][datos] (contrato en protocolos.h)
			t_paquete* contenido = recibir_paquete(socket_km);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			int len1;
			buffer_read(&len1, contenido->buffer, sizeof(int)); // LEEMOS CUANTO MIDE

			char* cadena_de_bytes = malloc(len1);				   // PEDIMOS EL ESPACIO JUSTO
			buffer_read(cadena_de_bytes, contenido->buffer, len1); // COPIAMOS EL CONTENIDO DEL BUFFER

			t_io* io = io_libre("STDOUT");

			if (io == NULL) {
				log_warning(ks_logger, "No hay IOs STDOUT libres para el proceso %d, se descarta el STDOUT", pid);
				free(cadena_de_bytes);
				eliminar_paquete(contenido);
				break; // return mataba el hilo de conexión con KM entero
			}

			enviar_stdout_io(io->fd, cadena_de_bytes, len1, pid);

			free(cadena_de_bytes);
			eliminar_paquete(contenido);
			break;
		}

		case OK_MEM_FREE: { // OK_MEM_FREE (PID)

			break;
		}

		case OK_MEM_ALLOC: { // OK_MEM_ALLOC (PID)

			break;
		}

		case OK_PROCESO_SUSPENDIDO: { // OK_PROCESO_SUSPENDIDO (PID)
			t_paquete* contenido = recibir_paquete(socket_km);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			eliminar_paquete(contenido);
			break;
		}

		case OK_PROCESO_DES_SUSPENDIDO: { // OK_PROCESO_DES_SUSPENDIDO (PID)
			t_paquete* contenido = recibir_paquete(socket_km);

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			eliminar_paquete(contenido);
			break;
		}

		case ERROR_CREACION_PROCESO: { // KM rechaza la creación: [pid]
			t_paquete* contenido = recibir_paquete(socket_km);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_creacion_pendiente* pendiente = sacar_creacion_pendiente(pid);
			if (pendiente != NULL) {
				log_error(ks_logger, "KM no pudo crear el proceso PID %d (%s)", pid, pendiente->path);
				free(pendiente->path);
				free(pendiente);
			} else {
				log_error(ks_logger, "KM no pudo crear el proceso PID %d (sin solicitud pendiente)", pid);
			}

			eliminar_paquete(contenido);
			break;
		}

		case OK_CREACION_PROCESO: { // KM confirma la creación: [pid]
			t_paquete* contenido = recibir_paquete(socket_km);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			t_creacion_pendiente* pendiente = sacar_creacion_pendiente(pid);
			if (pendiente == NULL) {
				log_warning(ks_logger, "KM confirmo la creacion del PID %d pero no habia solicitud pendiente", pid);
			} else {
				crear_proceso(pendiente->pid, pendiente->path, pendiente->prioridad);
				sem_post(&sem_ready); // hay un proceso nuevo en READY para el corto plazo
				free(pendiente->path);
				free(pendiente);
			}

			eliminar_paquete(contenido);
			break;
		}

		case RESPUESTA_DESSUSPENCION_LISTA: // Lista de PIDS Y PRIORIDADES dessuspendidas: [cantidad][pid1][prioridad1][pid2][prioridad2]...[pidN][prioridadN]
		{
			t_paquete* contenido = recibir_paquete(socket_km);

			int cantidad;
			buffer_read(&cantidad, contenido->buffer, sizeof(int));

			for (int i = 0; i < cantidad; i++) {
				int pid;
				buffer_read(&pid, contenido->buffer, sizeof(int));

				int prioridad;
				buffer_read(&prioridad, contenido->buffer, sizeof(int));

				ks_pasar_a_ready_desde_susp_ready_pid(pid, prioridad);
			}

			eliminar_paquete(contenido);
			break;
		}

		case -1: {
			log_warning(ks_logger, "[KM] se desconecto km");
			return NULL;
		}
		default: {
			log_warning(ks_logger, "[KM] operacion desconocida: %d", cod_op);
			break;
		}
		}
	}

	return NULL;
}