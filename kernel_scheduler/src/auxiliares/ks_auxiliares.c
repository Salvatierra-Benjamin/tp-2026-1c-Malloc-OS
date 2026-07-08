#include "ks_auxiliares.h"

#include "../main/ks_main.h"
#include "../mensajes/ks_mensajes.h"
#include "../planificadores/ks_planificador_mediano.h" // ks_timer_suspension
#include "../conexiones/ks_maneja_io.h" // ks_desbloquear_proceso
#include "../logs/ks_logs.h"            // log_mutex_tomado

void liberar_io(void* io_original) {
	t_io* io = (t_io*)io_original;
	free(io); // Liberar la memoria de la IO
}

void liberar_pedido_io(void* pedido_original) {
	t_pedido_io* pedido = (t_pedido_io*)pedido_original;
	free(pedido->datos); // NULL salvo STDOUT; free(NULL) es no-op
	free(pedido);
}

// Mapea la prioridad de un proceso al índice de cola VÁLIDO de array_queues.
// FIFO/RR: hay una sola cola (índice 0) y la prioridad no se usa para planificar.
// CMN: una cola por nivel; una prioridad fuera de rango se acota a la última cola
// (el PDF pide no planificar fuera de rango; acotar evita dejar el proceso sin cola
// y sin esto array_queues[ks_indice_cola(prioridad)] se iba de límites con las prioridades 1-3).
int ks_indice_cola(int prioridad) {
	if (cantidad_algoritmos <= 1 || prioridad < 0) {
		return 0;
	}
	if (prioridad >= cantidad_algoritmos) {
		return cantidad_algoritmos - 1;
	}
	return prioridad;
}

void liberar_cpu(void* cpu_original) {
	t_cpu* cpu = (t_cpu*)cpu_original;
	// NO liberar pcb_ejecutando acá: el PCB en ejecución también está en procs_exec
	// y se libera (y loguea su fin) al destruir esa lista. Liberarlo también acá
	// causaba use-after-free + doble free en el teardown de BSOD/shutdown.
	free(cpu); // Liberar solo la estructura del CPU
}

void liberar_proceso(void* proc) {
	t_pcb* proceso = (t_pcb*)proc;
	free(proceso->path); // Liberar la memoria del path
	free(proceso);		 // Liberar la memoria del PCB
}

void liberar_proceso_bsod(void* proc) {
	t_pcb* proceso = (t_pcb*)proc;
	log_fin_proceso(proceso, "Blue Screen of Death (BSOD)");
	free(proceso->path); // Liberar la memoria del path
	free(proceso);		 // Liberar la memoria del PCB
}

void matar_procesos_ready() {
	for (int i = 0; i < cantidad_algoritmos; i++) {
		queue_destroy_and_destroy_elements(array_queues[i].queue_ready_prioritario, liberar_proceso_bsod);
		queue_destroy_and_destroy_elements(array_queues[i].queue_ready, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_block, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_susp_block, liberar_proceso_bsod);
		list_destroy_and_destroy_elements(array_queues[i].list_susp_ready, liberar_proceso_bsod);
	}
}

void liberar_listas_matando_procesos_bsod() {
	pthread_mutex_lock(&mutex_cpus);
	list_destroy_and_destroy_elements(cpus, liberar_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	pthread_mutex_lock(&mutex_ios_sleep);
	list_destroy_and_destroy_elements(ios_sleep, liberar_io);
	pthread_mutex_unlock(&mutex_ios_sleep);

	pthread_mutex_lock(&mutex_ios_stdin);
	list_destroy_and_destroy_elements(ios_stdin, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdin);

	pthread_mutex_lock(&mutex_ios_stdout);
	list_destroy_and_destroy_elements(ios_stdout, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdout);

	queue_destroy_and_destroy_elements(pedidos_sleep, liberar_pedido_io);
	queue_destroy_and_destroy_elements(pedidos_stdin, liberar_pedido_io);
	queue_destroy_and_destroy_elements(pedidos_stdout, liberar_pedido_io);

	pthread_mutex_lock(&mutex_exec);
	list_destroy_and_destroy_elements(procs_exec, liberar_proceso_bsod);
	pthread_mutex_unlock(&mutex_exec);

	// procs_exit ya finalizaron y loguearon su fin con motivo EXIT: no re-loguear BSOD.
	pthread_mutex_lock(&mutex_exit);
	list_destroy_and_destroy_elements(procs_exit, liberar_proceso);
	pthread_mutex_unlock(&mutex_exit);
}

void liberar_listas_matando_procesos() {
	pthread_mutex_lock(&mutex_cpus);
	list_destroy_and_destroy_elements(cpus, liberar_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	pthread_mutex_lock(&mutex_ios_sleep);
	list_destroy_and_destroy_elements(ios_sleep, liberar_io);
	pthread_mutex_unlock(&mutex_ios_sleep);

	pthread_mutex_lock(&mutex_ios_stdin);
	list_destroy_and_destroy_elements(ios_stdin, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdin);

	pthread_mutex_lock(&mutex_ios_stdout);
	list_destroy_and_destroy_elements(ios_stdout, liberar_io);
	pthread_mutex_unlock(&mutex_ios_stdout);

	queue_destroy_and_destroy_elements(pedidos_sleep, liberar_pedido_io);
	queue_destroy_and_destroy_elements(pedidos_stdin, liberar_pedido_io);
	queue_destroy_and_destroy_elements(pedidos_stdout, liberar_pedido_io);

	pthread_mutex_lock(&mutex_exec);
	list_destroy_and_destroy_elements(procs_exec, liberar_proceso);
	pthread_mutex_unlock(&mutex_exec);

	pthread_mutex_lock(&mutex_exit);
	list_destroy_and_destroy_elements(procs_exit, liberar_proceso);
	pthread_mutex_unlock(&mutex_exit);
}

void liberar_mutex() {
	pthread_mutex_destroy(&mutex_cpus);
	pthread_mutex_destroy(&mutex_ios_sleep);
	pthread_mutex_destroy(&mutex_ios_stdin);
	pthread_mutex_destroy(&mutex_ios_stdout);
	pthread_mutex_destroy(&mutex_exec);
	// pthread_mutex_destroy(&mutex_block);
	// pthread_mutex_destroy(&mutex_susp_block);
	// pthread_mutex_destroy(&mutex_susp_ready);
	pthread_mutex_destroy(&mutex_exit);
	pthread_mutex_destroy(&mutex_array_queues);

	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_destroy(&array_queues[i].mutex_ready_prioritario);
		pthread_mutex_destroy(&array_queues[i].mutex_ready);
		pthread_mutex_destroy(&array_queues[i].mutex_block);
		pthread_mutex_destroy(&array_queues[i].mutex_susp_block);
		pthread_mutex_destroy(&array_queues[i].mutex_susp_ready);
	}
}

void inicializar_semaforos(void) {
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_cpu_libre, 0, 0);
	sem_init(&sem_memoria_disponible, 0, 0);
	sem_init(&sem_compactacion_finalizada, 0, 0);
}

void destruir_semaforos(void) {
	sem_destroy(&sem_ready);
	sem_destroy(&sem_cpu_libre);
	sem_destroy(&sem_memoria_disponible);
}
// Saca toda referencia al PCB de las colas de planificación. Barre TODAS las colas
// de prioridad (no solo la actual: la herencia pudo cambiar pcb->prioridad respecto
// de la cola donde quedó encolado). Idempotente: si no está, no hace nada.
void ks_remover_pcb_de_todas_las_colas(t_pcb* pcb) {
	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_lock(&array_queues[i].mutex_ready_prioritario);
		list_remove_element(array_queues[i].queue_ready_prioritario->elements, pcb);
		pthread_mutex_unlock(&array_queues[i].mutex_ready_prioritario);

		pthread_mutex_lock(&array_queues[i].mutex_ready);
		list_remove_element(array_queues[i].queue_ready->elements, pcb);
		pthread_mutex_unlock(&array_queues[i].mutex_ready);

		pthread_mutex_lock(&array_queues[i].mutex_block);
		list_remove_element(array_queues[i].list_block, pcb);
		pthread_mutex_unlock(&array_queues[i].mutex_block);

		pthread_mutex_lock(&array_queues[i].mutex_susp_block);
		list_remove_element(array_queues[i].list_susp_block, pcb);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_block);

		pthread_mutex_lock(&array_queues[i].mutex_susp_ready);
		list_remove_element(array_queues[i].list_susp_ready, pcb);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
	}
}

// Al finalizar un proceso: lo saca de la cola de espera de todo mutex, y si era
// dueño de alguno, se lo concede al siguiente en espera (o lo deja libre). Así no
// quedan mutexes trabados si un dueño muere/segfaultea con el lock tomado.
void ks_liberar_mutexes_de_proceso(t_pcb* pcb) {
	t_list* a_desbloquear = list_create();

	pthread_mutex_lock(&mutex_mutexes_kernel);
	for (int m = 0; m < list_size(mutexes_kernel); m++) {
		t_mutex_kernel* mtx = list_get(mutexes_kernel, m);
		pthread_mutex_lock(&mtx->mutex);

		list_remove_element(mtx->bloqueados->elements, pcb); // si esperaba, sacarlo

		if (mtx->duenio == pcb) {
			if (queue_is_empty(mtx->bloqueados)) {
				mtx->tomado = false;
				mtx->duenio = NULL;
			} else {
				t_pcb* siguiente = queue_pop(mtx->bloqueados);
				mtx->duenio = siguiente;
				log_mutex_tomado(siguiente, mtx->nombre);
				list_add(a_desbloquear, siguiente);
			}
		}
		pthread_mutex_unlock(&mtx->mutex);
	}
	pthread_mutex_unlock(&mutex_mutexes_kernel);

	// Desbloquear FUERA del lock de mutexes_kernel (ks_desbloquear_proceso toma
	// locks de colas) para no anidar mutex_mutexes_kernel con los de array_queues.
	for (int i = 0; i < list_size(a_desbloquear); i++) {
		ks_desbloquear_proceso((t_pcb*) list_get(a_desbloquear, i));
	}
	list_destroy(a_desbloquear);
}

bool finalizar_proceso(int pid, char* motivo) {
	t_pcb* pcb = buscar_pcb_por_pid(pid);
	if (pcb == NULL) {
		return false; // No se encontró el proceso con el PID dado
	}
	// Sacarlo de EXEC (venía ejecutando cuando pidió EXIT o segfaulteó); si no,
	// quedaba huérfano en procs_exec y lo tocaría la compactación.
	pthread_mutex_lock(&mutex_exec);
	list_remove_element(procs_exec, pcb);
	pthread_mutex_unlock(&mutex_exec);

	// >>> FIX BSOD: liberar mutexes del proceso y sacar toda referencia de las
	// colas ANTES de moverlo a procs_exit (queda referenciado solo ahí).
	ks_liberar_mutexes_de_proceso(pcb);
	ks_remover_pcb_de_todas_las_colas(pcb);
	// <<<

	pcb->estado = EXIT_ESTADO; // Cambia el estado del proceso a "EXIT"
	pthread_mutex_lock(&mutex_exit);
	list_add(procs_exit, pcb); // Agrega el proceso a la lista de procesos en estado EXIT
	pthread_mutex_unlock(&mutex_exit);

	log_fin_proceso(pcb, motivo);

	enviar_fin_proceso_km(pid);

	// Al terminar el proceso, KM libera su memoria → puede haber espacio para
	// des-suspender procesos que estaban en SUSP_READY. Disparo el mediano.
	sem_post(&sem_memoria_disponible);

	return true;
}

int asignar_pid(void) {
	pthread_mutex_lock(&mutex_siguiente_pid);
	int pid = siguiente_pid++;
	pthread_mutex_unlock(&mutex_siguiente_pid);
	return pid;
}

void registrar_creacion_pendiente(int pid, char* path, int prioridad) {
	t_creacion_pendiente* pendiente = malloc(sizeof(t_creacion_pendiente));
	pendiente->pid = pid;
	pendiente->path = strdup(path);
	pendiente->prioridad = prioridad;

	pthread_mutex_lock(&mutex_creaciones_pendientes);
	list_add(creaciones_pendientes, pendiente);
	pthread_mutex_unlock(&mutex_creaciones_pendientes);
}

t_creacion_pendiente* sacar_creacion_pendiente(int pid) {
	bool _tiene_pid(void* ptr) {
		return ((t_creacion_pendiente*)ptr)->pid == pid;
	}

	pthread_mutex_lock(&mutex_creaciones_pendientes);
	t_creacion_pendiente* pendiente = list_remove_by_condition(creaciones_pendientes, _tiene_pid);
	pthread_mutex_unlock(&mutex_creaciones_pendientes);

	return pendiente;
}

t_pcb* crear_proceso(int pid, char* path, int prioridad) {
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
	if (nuevo_pcb == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nuevo_pcb->pid = pid;
	nuevo_pcb->path = strdup(path);			   // Asegúrate de liberar esta memoria cuando ya no se necesite
	nuevo_pcb->prioridad = prioridad;
	nuevo_pcb->prioridad_original = prioridad; // Guardar la prioridad original para herencia de prioridad
	nuevo_pcb->cpu_fd = -1;					   // Inicialmente no asignado a ningún CPU
	nuevo_pcb->io_fd = -1;					   // Inicialmente no asignado a ningún IO
	nuevo_pcb->stdin_pendiente = false;
    nuevo_pcb->stdin_datos = NULL;
    nuevo_pcb->stdin_dir_logica = 0;

	nuevo_pcb->estado = NEW;

	log_creacion_proceso(nuevo_pcb);

	// CMN: NO se planifican procesos con prioridad fuera del rango de colas (spec:
	// "solamente se planificarán prioridades de la 0 a la N-1"). Queda en NEW, sin
	// encolar. (En FIFO/RR hay una sola cola y la prioridad no planifica, así que
	// no aplica: ks_indice_cola las colapsa a 0.)
	if (ks_config.algoritmo == CMN && (prioridad < 0 || prioridad >= cantidad_algoritmos)) {
		log_warning(ks_logger, "PID %d con prioridad %d fuera del rango de colas [0..%d]: NO se planifica (queda en NEW)", pid, prioridad, cantidad_algoritmos - 1);
		return nuevo_pcb;
	}

	log_cambio_estado(nuevo_pcb, NEW, READY);

	nuevo_pcb->estado = READY; // Estado inicial del proceso es "READY" (saltea NEW)

	pthread_mutex_lock(&array_queues[ks_indice_cola(prioridad)].mutex_ready);
	queue_push(array_queues[ks_indice_cola(prioridad)].queue_ready, nuevo_pcb); // Agrega el proceso a la cola correspondiente según su prioridad
	pthread_mutex_unlock(&array_queues[ks_indice_cola(prioridad)].mutex_ready);

	return nuevo_pcb;
}

t_cpu* crear_cpu(int id, int fd) {
	t_cpu* nueva_cpu = malloc(sizeof(t_cpu));
	if (nueva_cpu == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nueva_cpu->id = id;
	nueva_cpu->pcb_ejecutando = NULL; // Inicialmente no ejecutando ningún proceso
	nueva_cpu->fd = fd;

	return nueva_cpu;
}

t_io* crear_io(int fd, char* tipo_io) {
	t_io* nuevo_io = malloc(sizeof(t_io));
	if (nuevo_io == NULL) {
		return NULL; // Manejar error de asignación de memoria
	}

	nuevo_io->esta_libre = true;
	nuevo_io->fd = fd;
	nuevo_io->pid_pendiente = -1;
	nuevo_io->stdin_dir_logica = 0;
	nuevo_io->stdin_tamanio = 0;

	if (strcmp(tipo_io, "SLEEP") == 0) {
		pthread_mutex_lock(&mutex_ios_sleep);
		list_add(ios_sleep, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_sleep);
	} else if (strcmp(tipo_io, "STDIN") == 0) {
		pthread_mutex_lock(&mutex_ios_stdin);
		list_add(ios_stdin, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_stdin);
	} else if (strcmp(tipo_io, "STDOUT") == 0) {
		pthread_mutex_lock(&mutex_ios_stdout);
		list_add(ios_stdout, nuevo_io);
		pthread_mutex_unlock(&mutex_ios_stdout);
	}

	log_debug(ks_logger, "IO de tipo %s creada con fd %d", tipo_io, fd);

	return nuevo_io;
}

t_io* io_libre(char* tipo_io) {
	bool _io_esta_libre(void* ptr) {
		t_io* io = (t_io*)ptr;
		return io->esta_libre;
	}

	if (strcmp(tipo_io, "SLEEP") == 0) {
		pthread_mutex_lock(&mutex_ios_sleep);
		t_io* io = list_find(ios_sleep, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_sleep);
		return io;
	} else if (strcmp(tipo_io, "STDIN") == 0) {
		pthread_mutex_lock(&mutex_ios_stdin);
		t_io* io = list_find(ios_stdin, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_stdin);
		return io;
	} else if (strcmp(tipo_io, "STDOUT") == 0) {
		pthread_mutex_lock(&mutex_ios_stdout);
		t_io* io = list_find(ios_stdout, _io_esta_libre);
		if (io != NULL) {
			io->esta_libre = false;
		}
		pthread_mutex_unlock(&mutex_ios_stdout);
		return io;
	}
	return NULL;
}

// ------------------- Asignación / liberación de IO con cola de pedidos -------------------
// Si hay una IO libre del tipo, se la asigna y manda el pedido. Si no, encola el
// pedido; se despachará cuando una IO de ese tipo se libere (ks_liberar_io_*).
// El proceso ya debe estar en BLOCK antes de llamar a esto.

static bool _io_libre_pred(void* ptr) {
	return ((t_io*)ptr)->esta_libre;
}

void ks_pedir_io_sleep(int pid, int tiempo) {
	pthread_mutex_lock(&mutex_ios_sleep);
	t_io* io = list_find(ios_sleep, _io_libre_pred);
	if (io != NULL) {
		io->esta_libre = false;
		pthread_mutex_unlock(&mutex_ios_sleep);
		enviar_sleep_io(io->fd, tiempo, pid);
		return;
	}
	t_pedido_io* pedido = malloc(sizeof(t_pedido_io));
	*pedido = (t_pedido_io){.pid = pid, .tamanio = tiempo, .dir_logica = 0, .datos = NULL};
	queue_push(pedidos_sleep, pedido);
	pthread_mutex_unlock(&mutex_ios_sleep);
	log_debug(ks_logger, "No hay IO SLEEP libre: PID %d encolado", pid);
}

void ks_liberar_io_sleep(t_io* io) {
	pthread_mutex_lock(&mutex_ios_sleep);
	if (!queue_is_empty(pedidos_sleep)) {
		t_pedido_io* pedido = queue_pop(pedidos_sleep);
		pthread_mutex_unlock(&mutex_ios_sleep); // la IO sigue ocupada, la reasigno
		enviar_sleep_io(io->fd, pedido->tamanio, pedido->pid);
		free(pedido);
		return;
	}
	io->esta_libre = true;
	pthread_mutex_unlock(&mutex_ios_sleep);
}

void ks_pedir_io_stdin(int pid, uint32_t dir_logica, int tamanio) {
	pthread_mutex_lock(&mutex_ios_stdin);
	t_io* io = list_find(ios_stdin, _io_libre_pred);
	if (io != NULL) {
		io->esta_libre = false;
		io->pid_pendiente = pid;
		io->stdin_dir_logica = dir_logica;
		io->stdin_tamanio = tamanio;
		pthread_mutex_unlock(&mutex_ios_stdin);
		enviar_stdin_io(io->fd, tamanio, pid);
		return;
	}
	t_pedido_io* pedido = malloc(sizeof(t_pedido_io));
	*pedido = (t_pedido_io){.pid = pid, .tamanio = tamanio, .dir_logica = dir_logica, .datos = NULL};
	queue_push(pedidos_stdin, pedido);
	pthread_mutex_unlock(&mutex_ios_stdin);
	log_debug(ks_logger, "No hay IO STDIN libre: PID %d encolado", pid);
}

void ks_liberar_io_stdin(t_io* io) {
	pthread_mutex_lock(&mutex_ios_stdin);
	if (!queue_is_empty(pedidos_stdin)) {
		t_pedido_io* pedido = queue_pop(pedidos_stdin);
		io->pid_pendiente = pedido->pid;
		io->stdin_dir_logica = pedido->dir_logica;
		io->stdin_tamanio = pedido->tamanio;
		pthread_mutex_unlock(&mutex_ios_stdin);
		enviar_stdin_io(io->fd, pedido->tamanio, pedido->pid);
		free(pedido);
		return;
	}
	io->esta_libre = true;
	pthread_mutex_unlock(&mutex_ios_stdin);
}

void ks_pedir_io_stdout(int pid, void* datos, int len) {
	pthread_mutex_lock(&mutex_ios_stdout);
	t_io* io = list_find(ios_stdout, _io_libre_pred);
	if (io != NULL) {
		io->esta_libre = false;
		pthread_mutex_unlock(&mutex_ios_stdout);
		enviar_stdout_io(io->fd, datos, len, pid);
		return;
	}
	t_pedido_io* pedido = malloc(sizeof(t_pedido_io));
	pedido->pid = pid;
	pedido->tamanio = len;
	pedido->dir_logica = 0;
	pedido->datos = malloc(len); // copia propia: el buffer del caller se libera al volver
	memcpy(pedido->datos, datos, len);
	queue_push(pedidos_stdout, pedido);
	pthread_mutex_unlock(&mutex_ios_stdout);
	log_debug(ks_logger, "No hay IO STDOUT libre: PID %d encolado", pid);
}

void ks_liberar_io_stdout(t_io* io) {
	pthread_mutex_lock(&mutex_ios_stdout);
	if (!queue_is_empty(pedidos_stdout)) {
		t_pedido_io* pedido = queue_pop(pedidos_stdout);
		pthread_mutex_unlock(&mutex_ios_stdout);
		enviar_stdout_io(io->fd, pedido->datos, pedido->tamanio, pedido->pid);
		free(pedido->datos);
		free(pedido);
		return;
	}
	io->esta_libre = true;
	pthread_mutex_unlock(&mutex_ios_stdout);
}

void finalizar_cpu(t_cpu* cpu) {
	if (cpu->pcb_ejecutando != NULL) {
		cpu->pcb_ejecutando->cpu_fd = -1;
		cpu->pcb_ejecutando->io_fd = -1;
		cpu->pcb_ejecutando->estado = READY; // Cambia el estado del proceso a "READY" para que pueda ser planificado nuevamente

		log_cambio_estado(cpu->pcb_ejecutando, EXEC, READY);

		pthread_mutex_lock(&array_queues[ks_indice_cola(cpu->pcb_ejecutando->prioridad)].mutex_ready);
		queue_push(array_queues[ks_indice_cola(cpu->pcb_ejecutando->prioridad)].queue_ready, cpu->pcb_ejecutando); // Agrega el proceso de vuelta a la cola de ready correspondiente
		pthread_mutex_unlock(&array_queues[ks_indice_cola(cpu->pcb_ejecutando->prioridad)].mutex_ready);

		cpu->pcb_ejecutando = NULL;
	}

	pthread_mutex_lock(&mutex_cpus);
	bool _es_cpu(void* ptr) {
		t_cpu* aux = (t_cpu*)ptr;
		return aux->id == cpu->id;
	}
	list_remove_by_condition(cpus, _es_cpu);
	pthread_mutex_unlock(&mutex_cpus);

	free(cpu->pcb_ejecutando);
	free(cpu);
}

void ks_liberar_array_queues(void) {
	if (array_queues) {
		for (int i = 0; i < cantidad_algoritmos; i++) {
			queue_destroy(array_queues[i].queue_ready);
		}

		free(array_queues);
	}
}

t_pcb* buscar_pcb_por_pid(int pid) {
	bool _pcb_tiene_pid(void* ptr) {
		t_pcb* pcb = (t_pcb*)ptr;
		return pcb->pid == pid;
	}

	t_pcb* encontrado = NULL;

	for (int i = 0; i < cantidad_algoritmos; i++) {
		pthread_mutex_lock(&array_queues[i].mutex_ready);
		encontrado = list_find(array_queues[i].queue_ready->elements, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_ready);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_block);
		encontrado = list_find(array_queues[i].list_block, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_block);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_susp_block);
		encontrado = list_find(array_queues[i].list_susp_block, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_block);
		if (encontrado != NULL)
			return encontrado;

		pthread_mutex_lock(&array_queues[i].mutex_susp_ready);
		encontrado = list_find(array_queues[i].list_susp_ready, _pcb_tiene_pid);
		pthread_mutex_unlock(&array_queues[i].mutex_susp_ready);
		if (encontrado != NULL)
			return encontrado;
	}

	pthread_mutex_lock(&mutex_exec);
	encontrado = list_find(procs_exec, _pcb_tiene_pid);
	pthread_mutex_unlock(&mutex_exec);

	if (encontrado != NULL) {
		return encontrado;
	}

	// ptrhread_mutex_lock(&mutex_exit);
	// encontrado = list_find(procs_exit, _pcb_tiene_pid);
	// pthread_mutex_unlock(&mutex_exit);

	// no tiene sentido hacer el if, porque no me ahorro nada y retorna directamente el NULL si no lo encuentra

	return encontrado; // No se encontró el PCB con el PID dado
}

char* estado_a_string(estados estado) {
	switch (estado) {
	case NEW:
		return "NEW";
	case READY:
		return "READY";
	case EXEC:
		return "EXEC";
	case BLOCK:
		return "BLOCK";
	case SUSP_BLOCK:
		return "SUSP_BLOCK";
	case SUSP_READY:
		return "SUSP_READY";
	case EXIT_ESTADO:
		return "EXIT";
	default:
		return "UNKNOWN";
	}
}

void tomar_mutex_y_log(t_pcb* pcb, pthread_mutex_t* mutex, char* nombre_mutex) {
	log_mutex_tomado(pcb, nombre_mutex);
	pthread_mutex_lock(mutex);
}

void liberar_mutex_y_log(t_pcb* pcb, pthread_mutex_t* mutex, char* nombre_mutex) {
	pthread_mutex_unlock(mutex);
	log_mutex_liberado(pcb, nombre_mutex);
}

void ks_pasar_a_block(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_exec);

	bool _es_pcb(void* ptr) {
		t_pcb* aux = ptr;
		return aux->pid == pcb->pid;
	}

	list_remove_by_condition(procs_exec, _es_pcb);

	pthread_mutex_unlock(&mutex_exec);

	pcb->estado = BLOCK;
	pcb->tiempo_inicio_block = time(NULL);

	pthread_mutex_lock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_block);
	list_add(array_queues[ks_indice_cola(pcb->prioridad)].list_block, pcb);
	pthread_mutex_unlock(&array_queues[ks_indice_cola(pcb->prioridad)].mutex_block);

	log_cambio_estado(pcb, EXEC, BLOCK);

	// liberar la CPU que lo estaba ejecutando
	pthread_mutex_lock(&mutex_cpus);
	for (int i = 0; i < list_size(cpus); i++) {
		t_cpu* cpu = list_get(cpus, i);
		if (cpu->pcb_ejecutando == pcb) {
			cpu->pcb_ejecutando = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&mutex_cpus);
	pcb->cpu_fd = -1;

	// timer de mediano plazo: si sigue en BLOCK tras SUSPENSION_TIMEOUT pasa a SUSP_BLOCK
	pthread_t hilo_suspension;
	pthread_create(&hilo_suspension, NULL, ks_timer_suspension, pcb);
	pthread_detach(hilo_suspension);

	sem_post(&sem_cpu_libre);
}