#include "ks_maneja_km.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"
#include "../auxiliares/ks_auxiliares.h"

void *ks_conexion_km(void *_arg) {
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
			// case RESPUESTA_MEMORIA:
			//     ...
			//     break;
            case DETECCION_CORRUPCION_MEMORIA:
                matar_procesos_ready();
                liberar_listas();
                liberar_mutex();
                return NULL;
                // break;
            

		default:
			log_warning(ks_logger, "Operacion desconocida de KM (%d)", cod_op);
			t_paquete *basura = recibir_paquete(socket_km);
			if (basura)
				eliminar_paquete(basura);
			break;
		}
	}

	return NULL;
}