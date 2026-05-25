#include "ks_maneja_km.h"

#include "../main/ks_main.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"

void* ks_conexion_km(void* _arg) {
	(void)_arg;

	int socket_km = crear_conexion(ks_config.ip_km, ks_config.puerto_km);

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

	if (op != KM_OK_CONEXION) {
		log_error(ks_logger, "Handshake invalido. Esperaba KM_OK_CONEXION (%d), llegó %d", KM_OK_CONEXION, op);
		close(socket_km);
		return NULL;
	}

	log_conexion_km(); // Loguear conexión a Kernel Memory
	// log_debug(ks_logger, "Handshake con Kernel Memory OK");

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
			liberar_listas_matando_procesos();
			liberar_mutex();
			destruir_semaforos();
			return NULL;
			// break;
		}


		case AUMENTO_DE_MEMORIA: { // AUMENTO_DE_MEMORIA (TAMAÑO)
			t_paquete* contenido = recibir_paquete(fd);

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

		case OK_STDIN_DATOS_ESCRITOS: {

			break;
		}

		case STDOUT_RESTUESTA_LECTURA: { // STDOUT_RESTUESTA_LECTURA (BYTES)
			t_paquete* contenido = recibir_paquete(fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			eliminar_paquete(contenido);
			break;
		}

		case OK_SEGMENTO_ELIMINADO: {

			break;
		}

		case OK_PROCESO_SUSPENDIDO: { // OK_PROCESO_SUSPENDIDO (PID)
			t_paquete* contenido = recibir_paquete(fd);

			int pid;
			buffer_read(&pid, contenido->buffer, sizeof(int));

			eliminar_paquete(contenido);
			break;
		}

		case OK_PROCESO_DES_SUSPENDIDO: { // OK_PROCESO_DES_SUSPENDIDO (PID)
			t_paquete* contenido = recibir_paquete(fd);

			int tamanio;
			buffer_read(&tamanio, contenido->buffer, sizeof(int));

			eliminar_paquete(contenido);
			break;
		}

		case -1: {
			log_warning(ks_logger, "[KM] se desconecto km");
			ejecutando = 0;
			return NULL;
		}
		default: {
			log_warning(ks_logger, "[KM] operacion desconocida: %d", op_code);
			break;
		}
		}
	}

	return NULL;
}