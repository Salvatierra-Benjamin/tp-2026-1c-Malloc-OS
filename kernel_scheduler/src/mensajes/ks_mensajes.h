#ifndef KS_MENSAJES_H
#define KS_MENSAJES_H

#include "../estructuras/ks_estructuras.h"
#include <../../utils/src/utils/protocolos.h>
#include <commons/collections/list.h> // Necesario para t_list
#include <stdlib.h>
#include <string.h>

void enviar_ok_conexion(int fd);
void enviar_alojar_proceso_cpu(t_pcb* pcb);
void enviar_interrupcion_cpu(int cpu_fd, int pid);
void enviar_interrupcion_compactacion_cpu(int cpu_fd, int pid);
void enviar_sleep_io(int io_fd, int tiempo_ms, int pid);
void enviar_stdin_io(int io_fd, int tamanio, int pid);
void enviar_stdout_io(int io_fd, char* cadena_de_bytes, int len, int pid);
void enviar_solicitud_creacion_proceso_km(int pid, char* archivo_instrucciones, int prioridad);
void enviar_fin_proceso_km(int pid);
void enviar_confirmacion_desalojo_compactacion_km(void);
void enviar_bytes_stdin(int pid, uint32_t dir_logica, int tamanio, void* datos);
void enviar_solicitud_stdout_km(int pid, uint32_t dir_logica, int tamanio);
void enviar_aviso_suspension_km(int pid);
void enviar_lista_desuspension_km(t_list* lista_a_desuspender);
void enviar_mem_alloc(int id_segmento, int tamanio, int pid);
void enviar_mem_free(int id_segmento, int pid);

#endif