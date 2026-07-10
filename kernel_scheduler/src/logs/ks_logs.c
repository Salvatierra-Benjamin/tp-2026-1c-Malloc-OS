#include "ks_logs.h"

#include "../auxiliares/ks_auxiliares.h" // estado_a_string
#include "../main/ks_main.h"
#include "../planificadores/ks_planificador_corto.h" // ks_verificar_desalojo_prioridad

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <commons/string.h>

#include <../../utils/src/utils/protocolos.h>

// En caso de que fallen los logs, se puede usar: pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER; y ponerlo en cada función de logueo para evitar que se mezclen los mensajes. Pero lo ideal es que no fallen los logs, así que no lo hice.

// Conexión a Kernel Memory: “## Conectado a Kernel Memory”
void log_conexion_km()
{
	log_info(ks_logger, "## Conectado a Kernel Memory");
}

// Conexión de CPU: “## CPU <ID CPU> Conectada”
void log_conexion_cpu(int id_cpu)
{
	log_info(ks_logger, "## CPU %d Conectada", id_cpu);
}

// Conexión de IO
// void log_conexion_io(t_io* io) {
// 	log_info(ks_logger, "IO Conectada, path de IO: %s - prioridad: %d", io->path, io->prioridad);
// }

// Creación de Proceso: “## (<PID>) Se crea el proceso - Estado: NEW”
void log_creacion_proceso(t_pcb *pcb)
{
	log_info(ks_logger, "## (%d) Se crea el proceso - Estado: NEW", pcb->pid);
}

// Syscall recibida: “## (<PID>) - Solicitó syscall: <NOMBRE_SYSCALL>”
void log_syscall_recibida(t_pcb *pcb, char *nombre_syscall)
{
	log_info(ks_logger, "## (%d) - Solicitó syscall: %s", pcb->pid, nombre_syscall);
}

// Cambio de Estado: “## (<PID>) Pasa del estado <ESTADO_ANTERIOR> al estado <ESTADO_ACTUAL>”
void log_cambio_estado(t_pcb *pcb, estados estado_anterior, estados estado_actual)
{
	log_info(ks_logger, "## (%d) Pasa del estado %s al estado %s", pcb->pid, estado_a_string(estado_anterior), estado_a_string(estado_actual));

	// Preempción por prioridad: solo cuando un proceso LLEGA a READY (arribo desde
	// NEW/BLOCK/SUSP_READY), no cuando se lo desaloja de EXEC (eso no es un arribo y
	// causaría re-chequeos en cadena). El gate de algoritmo (CMN) y queue_preemption
	// está dentro de ks_verificar_desalojo_prioridad.
	if (estado_actual == READY && estado_anterior != EXEC)
	{
		ks_verificar_desalojo_prioridad(pcb);
	}

	// Hacer en cada lugar donde se llame a la funcion, no acá (por la compactación)
	// switch (estado_actual) {
	// case READY:
	// 	sem_post(&sem_ready);
	// 	break;

	// case EXIT:
	// 	sem_post(&sem_memoria_disponible);
	// 	break;

	// default:
	// 	break;
	// }
}

// Fin de IO: “## (<PID>) finalizó IO y pasa a READY / SUSP. READY”
void log_fin_io(t_pcb *pcb)
{
	log_info(ks_logger, "## (%d) finalizó IO y pasa a READY / SUSP. READY", pcb->pid);
}

// Mutex Tomado: “## (<PID>) Toma el Mutex <NOMBRE_MUTEX>”
void log_mutex_tomado(t_pcb *pcb, char *nombre_mutex)
{
	log_info(ks_logger, "## (%d) Toma el Mutex %s", pcb->pid, nombre_mutex);
}

// Mutex Liberado: “## (<PID>) Libera el Mutex <NOMBRE_MUTEX>”
void log_mutex_liberado(t_pcb *pcb, char *nombre_mutex)
{
	log_info(ks_logger, "## (%d) Libera el Mutex %s", pcb->pid, nombre_mutex);
}

// Cambio de prioridad de Proceso: “## <PID> Cambio de prioridad: <PRIORIDAD_ANTERIOR> - <PRIORIDAD_NUEVA>”
void log_cambio_prioridad(t_pcb *pcb, int prioridad_anterior)
{
	log_info(ks_logger, "## %d Cambio de prioridad: %d - %d", pcb->pid, prioridad_anterior, pcb->prioridad);
}

// Desalojo por fin de Quantum: “## (<PID>) - Desalojado por fin de quantum”
void log_desalojo_quantum(t_pcb *pcb)
{
	log_info(ks_logger, "## (%d) - Desalojado por fin de quantum", pcb->pid);
}

// Desalojo por Cola más prioritaria: “## (<PID>) Prioridad: <PRIORIDAD_DESALOJADO> - Desalojado por cola más prioritaria por el proceso <PID> con prioridad <PRIORIDAD_NUEVA>”
void log_desalojo_prioridad(t_pcb *pcb_desalojado, t_pcb *pcb_nuevo)
{
	log_info(ks_logger, "## (%d) Prioridad: %d - Desalojado por cola más prioritaria por el proceso %d con prioridad %d",
			 pcb_desalojado->pid, pcb_desalojado->prioridad, pcb_nuevo->pid, pcb_nuevo->prioridad);
}

// Inicio compactación: “## Inicio de compactación”
void log_inicio_compactacion()
{
	log_info(ks_logger, "## Inicio de compactación");
}

// Fin compactación: “## Fin de compactación”
void log_fin_compactacion()
{
	log_info(ks_logger, "## Fin de compactación");
}

// Fin de Proceso: “## (<PID>) finalizó su ejecución con motivo de <MOTIVO>”
void log_fin_proceso(t_pcb *pcb, char *motivo)
{
	log_info(ks_logger, "## (%d) finalizó su ejecución con motivo de %s", pcb->pid, motivo);
}
