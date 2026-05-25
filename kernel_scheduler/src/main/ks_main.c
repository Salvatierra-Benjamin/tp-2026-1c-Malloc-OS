#include "ks_main.h"

#include "../conexiones/ks_maneja_cpu.h"
#include "../conexiones/ks_maneja_io.h"
#include "../conexiones/ks_maneja_km.h"

#include "../auxiliares/ks_auxiliares.h"
#include "../logs/ks_logs.h"

#include "../planificadores/ks_planificador_corto.h"
#include "../planificadores/ks_planificador_largo.h"
#include "../planificadores/ks_planificador_mediano.h"

#include "../mensajes/ks_mensajes.h"

static algoritmos ks_algoritmo_from_string(char* str) {
	if (strcmp(str, "FIFO") == 0)
		return FIFO;
	if (strcmp(str, "RR") == 0)
		return RR;
	if (strcmp(str, "CMN") == 0)
		return CMN;

	return INVALIDO;
}

void* ks_parsear_algoritmos(char* raw) {
	if (!raw) {
		cantidad_algoritmos = 0;
		array_queues = NULL;
		return;
	}

	// saco corchetes
	char* limpio = string_duplicate(raw);
	string_replace(&limpio, "[", "");
	string_replace(&limpio, "]", "");

	// separo por coma
	char** tokens = string_split(limpio, ",");

	// contar algoritmos
	cantidad_algoritmos = 0;
	while (tokens[cantidad_algoritmos] != NULL) {
		cantidad_algoritmos++;
	}

	// reservar memoria para array global
	array_queues = malloc(sizeof(t_elemento_array_algoritmos) * cantidad_algoritmos);

	// llenar array
	for (int i = 0; i < cantidad_algoritmos; i++) {
		string_trim(&tokens[i]);
		array_queues[i].alg = ks_algoritmo_from_string(tokens[i]);

		// crear queue vacía asociada con su mutex correspondiente
		array_queues[i].queue_ready_prioritario = queue_create();
		pthread_mutex_init(&array_queues[i].mutex_ready_prioritario, NULL);

		array_queues[i].queue_ready = queue_create();
		pthread_mutex_init(&array_queues[i].mutex_ready, NULL);

		array_queues[i].list_block = list_create();
		pthread_mutex_init(&array_queues[i].mutex_block, NULL);

		array_queues[i].list_susp_block = list_create();
		pthread_mutex_init(&array_queues[i].mutex_susp_block, NULL);

		array_queues[i].queue_susp_ready = queue_create();
		pthread_mutex_init(&array_queues[i].mutex_susp_ready, NULL);
	}

	// liberar auxiliares
	string_array_destroy(tokens);
	free(limpio);
}

bool ks_cargar_config(char* path_config, char* path_proceso_inicial) {
	t_config* config = config_create(path_config);

	if (!config) {
		return false;
	}

	ks_config.ip_km = strdup(config_get_string_value(config, "IP_KM"));
	ks_config.puerto_km = strdup(config_get_string_value(config, "PUERTO_KM"));
	ks_config.puerto_escucha =
		strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
	ks_config.algoritmo = ks_algoritmo_from_string(
		config_get_string_value(config, "PLANIFICATION_ALGORITHM"));
	ks_config.algoritmo_queues = ks_parsear_algoritmos(
		config_get_string_value(config, "QUEUES_ALGORITHMS"));
	ks_config.rr_quantum = config_get_int_value(config, "RR_QUANTUM");
	config_get_string_value(config, "QUEUE_PREEMPTION");
	// ks_config.queue_preemption = strdup(config_get_string_value(config,
	// "QUEUE_PREEMPTION"));
	if (strdup(config_get_string_value(config, "QUEUE_PREEMPTION")) == "TRUE") {
		ks_config.queue_preemption = true;
	} else {
		ks_config.queue_preemption = false;
	}

	ks_config.suspension_timeout =
		config_get_int_value(config, "SUSPENSION_TIMEOUT");
	ks_config.path_proceso_inicial = strdup(path_proceso_inicial);
	const char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
	t_log_level level = log_level_from_string((char*)log_level_string);

	ks_logger =
		log_create("kernel_scheduler.log", "KERNEL_SCHEDULER", 1, level);

	config_destroy(config);

	/* // Leer la lista de algoritmo queues
	for (int i = 0; i < list_size(ks_config.algoritmo_queues); i++) {
	algoritmos* alg = list_get(ks_config.algoritmo_queues, i);

		switch (*alg) {
			case FIFO:
				printf("FIFO\n");
				break;
			case RR:
				printf("RR\n");
				break;
		}
	}
	*/
	// PARA DEBUGGEAR
	return ks_logger != NULL;
}

void crear_listas() {
	cpus = list_create();
	ios = list_create();
	// procesos = list_create();

	procs_exec = list_create();
	// procs_block = list_create();
	// procs_susp_block = list_create();
	// procs_susp_ready = list_create();
	procs_exit = list_create();
}

void ks_liberar_config(void) {
	if (ks_logger) {
		log_destroy(ks_logger);
	}
}

void* ks_conexion_loop(void* _arg) {
	(void)_arg;

	printf("Entra en ks_conexion_loop");

	for (;;) {
		int fd = esperar_cliente(ks_server_fd);
		if (fd < 0) {
			log_error(ks_logger, "accept falló");
			continue;
		}

		log_debug(ks_logger, "Se conectó un cliente en el socket: %d", fd);

		int op = recibir_operacion(fd);
		if (op <= 0) {
			close(fd);
			continue;
		}

		if (op == HANDSHAKE_IO) {
			log_debug(ks_logger, "Se conecto un nuevo IO");
			// NO recibir_paquete aca
			// enviar_ks_ok_conexion(fd);
			pthread_t hilo_io;

			int* socket_ptr = malloc(sizeof(int));
			if (socket_ptr == NULL) {
				close(fd);
				return;
			}
			*socket_ptr = fd;

			if (pthread_create(&hilo_io, NULL, ks_maneja_io, socket_ptr) != 0) {
				log_error(ks_logger, "no se inicia el hilo io");
				free(socket_ptr);
				close(fd);
				return;
			}
			pthread_detach(hilo_io);
			log_debug(ks_logger, "Se detachea el hilo_io");
		} else if (op == HANDSHAKE_CPU) {
			log_debug(ks_logger, "Se conecto un nuevo CPU");

			enviar_ok_conexion_cpu(fd);

			int* socket_ptr = malloc(sizeof(int));
			*socket_ptr = fd;
			if (socket_ptr == NULL) {
				close(fd);
				return;
			}

			t_paquete* contenido = recibir_paquete(fd);

			int id_cpu = NULL;
			buffer_read(&id_cpu, contenido->buffer, sizeof(char*));

			t_cpu* cpu = crear_cpu(id_cpu, socket_ptr);
			eliminar_paquete(contenido);

			log_conexion_cpu(id_cpu);

			// NO recibir_paquete aca
			pthread_t hilo_cpu;

			if (pthread_create(&hilo_cpu, NULL, ks_maneja_cpu, cpu) != 0) {
				log_error(ks_logger, "no se inicia el hilo cpu");
				free(socket_ptr);
				close(fd);
				return;
			}
			pthread_detach(hilo_cpu);
			log_debug(ks_logger, "Se detachea el hilo_cpu");
		}

		else {
			log_error(ks_logger, "Handshake desconocido (%d)", op);
			// Si vino algo, descartalo
			t_paquete* trash = recibir_paquete(fd);
			if (trash)
				eliminar_paquete(trash);
			close(fd);
			continue;
		}
	}

	return NULL;
}

static void sigint_handler(int _sig) {
	(void)_sig;

	if (ks_logger)
		log_debug(ks_logger, "SIGINT: cerrando Kernel Scheduler...");

	if (ks_server_fd != -1)
		close(ks_server_fd);

	ks_liberar_config();

	exit(0);
}

int main(int argc, char* argv[]) {			   //  ~ ./bin/kernel_scheduler [Archivo Config]
											   //  [Path Proceso Inicial]
	if (!ks_cargar_config(argv[1], argv[2])) { //! Está hardcodeado el codigo del config
		fprintf(stderr, "No pude cargar ks.config (busca en el CWD).\n");

		return EXIT_FAILURE;
	}

	crear_listas();
	inicializar_semaforos();

	log_debug(ks_logger, "Empezando la conexion KS");

	signal(SIGINT, sigint_handler);

	pthread_t hilo_km;
	pthread_create(&hilo_km, NULL, ks_conexion_km, NULL);
	pthread_detach(hilo_km);

	// ABRIR SERVIDOR DE ESCUCHA PARA CPU E IO
	ks_server_fd = iniciar_servidor(ks_config.puerto_escucha);
	if (ks_server_fd < 0) {
		log_error(ks_logger, "No pude iniciar servidor en %s", ks_config.puerto_escucha);
		return EXIT_FAILURE;
	}

	log_debug(ks_logger, "Kernel Scheduler escuchando en puerto %s", ks_config.puerto_escucha);

	pthread_t hilo_conexiones;
	pthread_create(&hilo_conexiones, NULL, ks_conexion_loop, NULL);
	pthread_detach(hilo_conexiones);

	// dormir el main para siempre
	for (;;)
		pause();

	return 0;
}
