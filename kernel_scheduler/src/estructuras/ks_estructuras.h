#ifndef KS_ESTRUCTURAS_H
#define KS_ESTRUCTURAS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <time.h>

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
} t_io;

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

// ESTADOS
typedef enum {
	NEW = 0,
	READY = 1,
	EXEC = 2,
	BLOCK = 3,
	SUSP_BLOCK = 4,
	SUSP_READY = 5,
	EXIT = 6
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
} t_pcb;

typedef struct {
	char* nombre;
	bool tomado;
	t_pcb* duenio;
	t_queue* bloqueados;
	pthread_mutex_t mutex;
} t_mutex_kernel;

// VARIABLES GLOBALES
extern t_log* ks_logger;
extern t_ks_config ks_config;

extern int ks_server_fd;

extern t_elemento_array_algoritmos* array_queues;
extern int cantidad_algoritmos;

extern bool compactacion_en_progreso;

// LISTAS
extern t_list* cpus;
extern t_list* ios_sleep;
extern t_list* ios_stdin;
extern t_list* ios_stdout;

extern t_list* procs_exec;
// extern t_list* procs_block;
// extern t_list* procs_susp_block;
// extern t_list* procs_susp_ready;
extern t_list* procs_exit;

extern t_list* mutexes_kernel;

// MUTEX
extern pthread_mutex_t mutex_cpus;
extern pthread_mutex_t mutex_ios_sleep;
extern pthread_mutex_t mutex_ios_stdin;
extern pthread_mutex_t mutex_ios_stdout;

extern pthread_mutex_t mutex_exec;
extern pthread_mutex_t mutex_exit;

extern pthread_mutex_t mutex_array_queues;

extern pthread_mutex_t mutex_mutexes_kernel;

extern pthread_mutex_t mutex_compactacion;

// SEMAFOROS
extern sem_t sem_ready;
extern sem_t sem_cpu_libre;
extern sem_t sem_memoria_disponible;
extern sem_t sem_compactacion_finalizada;

#endif