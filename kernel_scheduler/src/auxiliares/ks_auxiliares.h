#ifndef KS_AUXILIARES_H
#define KS_AUXILIARES_H

#include "../estructuras/ks_estructuras.h"
#include "../logs/ks_logs.h"

#include <stdlib.h>
#include <string.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

// LIBERACION
void liberar_proceso(void* proc);
void liberar_proceso_bsod(void* proc);

void matar_procesos_ready(void);

void liberar_listas(void);
void liberar_mutex(void);

void ks_liberar_array_queues(void);

// PCB
int asignar_pid(void);
void registrar_creacion_pendiente(int pid, char* path, int prioridad);
t_creacion_pendiente* sacar_creacion_pendiente(int pid);

bool finalizar_proceso(int pid);

t_pcb* crear_proceso(int pid, char* path, int prioridad);

t_pcb* buscar_pcb_por_pid(int pid);

void ks_pasar_a_block(t_pcb* pcb);

char* estado_a_string(estados estado);

// CPU / IO
t_cpu* crear_cpu(int id, int fd);
void finalizar_cpu(t_cpu* cpu);
t_io* crear_io(int fd, char* tipo_io);
t_io* io_libre(char* tipo_io);

int ks_indice_cola(int prioridad);

void ks_pedir_io_sleep(int pid, int tiempo);
void ks_liberar_io_sleep(t_io* io);
void ks_pedir_io_stdin(int pid, uint32_t dir_logica, int tamanio);
void ks_liberar_io_stdin(t_io* io);
void ks_pedir_io_stdout(int pid, void* datos, int len);
void ks_liberar_io_stdout(t_io* io);

// SEMAFOROS
void inicializar_semaforos(void);
void destruir_semaforos(void);

void liberar_listas_matando_procesos_bsod(void);

#endif