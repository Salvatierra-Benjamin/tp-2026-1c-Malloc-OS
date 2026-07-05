#include "ks_main.h"

// VARIABLES GLOBALES
t_log* ks_logger = NULL;
t_ks_config ks_config;

int ks_server_fd = -1;
int socket_km = -1;

// ! 0 ES LA MAYOR PRIORIDAD!!!!, LUEGO LE VAN SIGUIENDO EN ORDEN LOS VALORES (CUANTO MAYOR EL VALOR, MENOR LA PRIORIDAD)
t_elemento_array_algoritmos* array_queues = NULL;
int cantidad_algoritmos = 0;

bool compactacion_en_progreso = false;

// LISTAS
t_list* cpus = NULL;
t_list* ios_sleep = NULL;
t_list* ios_stdin = NULL;
t_list* ios_stdout = NULL;

t_list* procs_exec = NULL;
// t_list *procs_block = NULL;
// t_list *procs_susp_block = NULL;
// t_list *procs_susp_ready = NULL;
t_list* procs_exit = NULL;

t_list* mutexes_kernel = NULL;

t_list* creaciones_pendientes = NULL;
int siguiente_pid = 0;

// MUTEX
pthread_mutex_t mutex_cpus = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ios_sleep = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ios_stdin = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ios_stdout = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_exit = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_array_queues = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_mutexes_kernel = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_creaciones_pendientes = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_siguiente_pid = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_compactacion = PTHREAD_MUTEX_INITIALIZER;

// SEMAFOROS
sem_t sem_ready;
sem_t sem_cpu_libre;
sem_t sem_memoria_disponible;
sem_t sem_compactacion_finalizada;