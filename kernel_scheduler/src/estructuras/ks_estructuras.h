#ifndef KS_ESTRUCTURAS_H
#define KS_ESTRUCTURAS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <time.h>

// ESTADOS
typedef enum {
	NEW = 0,
	READY = 1,
	EXEC = 2,
	BLOCK = 3,
	SUSP_BLOCK = 4,
	SUSP_READY = 5,
	EXIT_ESTADO = 6 // "EXIT" colisiona con la instruccion EXIT de protocolos.h
} estados;

// PCB
typedef struct {
	int pid;
	char* path;
	int prioridad;
	int prioridad_original;
	int cpu_fd;
	int io_fd;
	estados estado;
	time_t tiempo_inicio_block;
	time_t tiempo_inicio_suspendido;
	bool   stdin_pendiente;      // init false en crear_pcb
    void*  stdin_datos;          // malloc propio de KS; free tras escribir
    uint32_t stdin_dir_logica;
    int    stdin_tamanio;
} t_pcb;

// CPU
typedef struct {
	int id;
	t_pcb* pcb_ejecutando; // TODO: agregarlo cuando se pasa a exec, sacarlo cuando se bloquea o termina
	int fd;
} t_cpu;

// IO
typedef struct {
	bool esta_libre;
	int fd;
	// Pedido de STDIN en curso en esta IO: OK_STDIN trae solo los datos, así que
	// acá recuerdo a quién y dónde escribirlos cuando la IO responda.
	int pid_pendiente;
	uint32_t stdin_dir_logica;
	int stdin_tamanio;
} t_io;

// PEDIDO DE IO ENCOLADO (cuando no hay IO libre del tipo pedido)
typedef struct {
	int pid;
	int tamanio;		 // SLEEP: tiempo en ms; STDIN/STDOUT: cantidad de bytes
	uint32_t dir_logica; // STDIN: dónde escribir lo leído (SLEEP/STDOUT no lo usan)
	void* datos;		 // STDOUT: bytes a imprimir (malloc propio); resto NULL
} t_pedido_io;

// ENUM ALGORITMOS
typedef enum { FIFO = 1,
			   RR = 2,
			   CMN = 3,
			   INVALIDO = 0 } algoritmos;

// ARRAY DE ALGORITMOS
typedef struct {
	algoritmos alg;
	// ready prioritario por desalojo por compactacion
	t_queue* queue_ready_prioritario;
	pthread_mutex_t mutex_ready_prioritario;
	// ready
	t_queue* queue_ready;
	pthread_mutex_t mutex_ready;
	// block
	t_list* list_block;
	pthread_mutex_t mutex_block;
	// susp_block
	t_list* list_susp_block;
	pthread_mutex_t mutex_susp_block;
	// susp_ready // Era queue, pero no se puede buscar para dessuspender, asi que lo cambio a lista
	t_list* list_susp_ready;
	pthread_mutex_t mutex_susp_ready;
} t_elemento_array_algoritmos;

// CONFIG
typedef struct {
	char* ip_km;
	char* puerto_km;
	char* puerto_escucha;
	algoritmos algoritmo;
	bool queue_preemption;
	int rr_quantum;
	int suspension_timeout;
	char* path_proceso_inicial;
	t_log_level log_level;
} t_ks_config;

typedef struct {
	char* nombre;
	bool tomado;
	t_pcb* duenio;
	t_queue* bloqueados;
	pthread_mutex_t mutex;
} t_mutex_kernel;

// CREACION PENDIENTE (solicitud enviada a KM, esperando OK/ERROR_CREACION_PROCESO)
typedef struct {
	int pid;
	char* path;
	int prioridad;
} t_creacion_pendiente;

// VARIABLES GLOBALES
extern t_log* ks_logger;
extern t_ks_config ks_config;

extern int ks_server_fd;
extern int socket_km;

extern t_elemento_array_algoritmos* array_queues;
extern int cantidad_algoritmos;

extern bool compactacion_en_progreso;
extern int cpus_pendientes_desalojo_compactacion;

// LISTAS
extern t_list* cpus;
extern t_list* ios_sleep;
extern t_list* ios_stdin;
extern t_list* ios_stdout;

// Pedidos encolados por tipo cuando no hay IO libre (protegidos por mutex_ios_*)
extern t_queue* pedidos_sleep;
extern t_queue* pedidos_stdin;
extern t_queue* pedidos_stdout;

extern t_list* procs_exec;
// extern t_list* procs_block;
// extern t_list* procs_susp_block;
// extern t_list* procs_susp_ready;
extern t_list* procs_exit;

// PCBs con STDIN diferido tras des-suspensión: se sacan de susp_ready y esperan el
// OK_STDIN_DATOS_ESCRITOS de KM para recién ahí pasar a READY. Viven fuera de las
// colas normales, por eso necesitan su propia lista (si no, quedan huérfanos).
extern t_list* pendientes_stdin_post_desusp;

extern t_list* mutexes_kernel;

extern t_list* creaciones_pendientes;
extern int siguiente_pid;

// MUTEX
extern pthread_mutex_t mutex_cpus;
extern pthread_mutex_t mutex_ios_sleep;
extern pthread_mutex_t mutex_ios_stdin;
extern pthread_mutex_t mutex_ios_stdout;

extern pthread_mutex_t mutex_exec;
extern pthread_mutex_t mutex_exit;

extern pthread_mutex_t mutex_array_queues;

extern pthread_mutex_t mutex_mutexes_kernel;

extern pthread_mutex_t mutex_creaciones_pendientes;
extern pthread_mutex_t mutex_siguiente_pid;

extern pthread_mutex_t mutex_compactacion;

extern pthread_mutex_t mutex_pendientes_stdin;

// SEMAFOROS
extern sem_t sem_ready;
extern sem_t sem_cpu_libre;
extern sem_t sem_memoria_disponible;
extern sem_t sem_compactacion_finalizada;

#endif