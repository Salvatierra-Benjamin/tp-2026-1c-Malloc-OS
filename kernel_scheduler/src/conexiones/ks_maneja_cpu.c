#include "ks_maneja_cpu.h"

#include "../main/ks_main.h"
#include "../logs/ks_logs.h"

void* ks_maneja_cpu(void* arg) {
    int fd = *(int*)arg;
    free(arg);

    log_debug(ks_logger, "Entra en ks_maneja_cpu (fd = %d)", fd);
    log_conexion_cpu(fd);

    int ejecutando = 1;

    while (ejecutando)
    {
        int op_code = recibir_operacion(fd);
        switch (op_code) {
            case MUTEX_CREATE: // MUTEX_CREATE (Nombre Mutex)
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* nombre_mutex = NULL;
                buffer_read(&nombre_mutex, contenido->buffer, sizeof(char*));

                t_mutex_kernel* mutex = malloc(sizeof(t_mutex_kernel));
                
                mutex->nombre = strdup(nombre);
                mutex->tomado = false;
                mutex->duenio = NULL;
                mutex->bloqueados = queue_create();
                pthread_mutex_init(&mutex->mutex, NULL);

                list_add(

                    mutexes_kernel,

                    mutex

                );

                free(nombre_mutex);
                eliminar_paquete(contenido);
                break;
            }

            case MUTEX_LOCK: // MUTEX_LOCK (Nombre Mutex)
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* nombre_mutex = NULL;
                buffer_read(&nombre_mutex, contenido->buffer, sizeof(char*));

                t_mutex_kernel* mutex = ks_buscar_mutex(nombre_mutex);

                if (mutex == NULL) {
                    return;
                }

                pthread_mutex_lock(&mutex->mutex);

                if (!mutex->tomado) {

                    mutex->tomado = true;
                    mutex->duenio = pcb;

                    log_mutex_tomado(pcb, nombre_mutex);

                    pthread_mutex_unlock(&mutex->mutex);

                    return;
                }

                // HERENCIA PRIORIDAD

                if (pcb->prioridad < mutex->duenio->prioridad) {
                    mutex->duenio->prioridad = pcb->prioridad;
                }

                queue_push(mutex->bloqueados, pcb);
                pthread_mutex_unlock(&mutex->mutex);

                ks_pasar_a_block(pcb);

                free(nombre_mutex);
                eliminar_paquete(contenido);
                break;
            }

            case MUTEX_UNLOCK: // MUTEX_UNLOCK (Nombre Mutex)
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* nombre_mutex = NULL;
                buffer_read(&nombre_mutex, contenido->buffer, sizeof(char*));

                t_mutex_kernel* mutex = ks_buscar_mutex(nombre_mutex);

                if (mutex == NULL) {
                    return;
                }

                pthread_mutex_lock(&mutex->mutex);

                if (mutex->duenio != pcb) {
                    pthread_mutex_unlock(&mutex->mutex);
                    return;
                }

                pcb->prioridad = pcb->prioridad_original;

                log_mutex_liberado(pcb, nombre_mutex);

                if (queue_is_empty(mutex->bloqueados)) {

                    mutex->tomado = false;
                    mutex->duenio = NULL;

                    pthread_mutex_unlock(&mutex->mutex);
                    return;
                }

                t_pcb* desbloqueado = queue_pop(mutex->bloqueados);
                mutex->duenio = desbloqueado;
                log_mutex_tomado(desbloqueado, nombre_mutex);
                pthread_mutex_unlock(&mutex->mutex);
                
                // BLOCK -> READY
                desbloqueado->estado = READY;
                pthread_mutex_lock(&array_queues[desbloqueado->prioridad].mutex_ready);
                queue_push(array_queues[desbloqueado->prioridad].queue_ready, desbloqueado);
                pthread_mutex_unlock(&array_queues[desbloqueado->prioridad].mutex_ready);

                free(nombre_mutex);
                eliminar_paquete(contenido);
                break;
            }

            case MEM_ALLOC: // MEM_ALLOC (Id del Segmento, Tamaño)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int id_segmento;
                buffer_read(&id_segmento, contenido->buffer, sizeof(int));

                int tamanio;
                buffer_read(&tamanio, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }

            case MEM_FREE: // MEM_FREE (Id del Segmento)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int id_segmento;
                buffer_read(&id_segmento, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }

            case SLEEP: // SLEEP (Tiempo)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int tiempo;
                buffer_read(&tiempo, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }
            
            case INIT_PROC: // INIT_PROC (Archivo de instrucciones, Prioridad)
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* archivo_insrucciones = NULL;
                buffer_read(&archivo_insrucciones, contenido->buffer, sizeof(char*));

                int prioridad;
                buffer_read(&prioridad, contenido->buffer, sizeof(int));

                free(archivo_insrucciones);
                eliminar_paquete(contenido);
                break;
            }

            case EXIT: 
            {
                
                break;
            }

            case OK_INTERRUPCION: // OK_INTERRUPCION (PID, MOTIVO DE INTERRUPCION)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int pid;
                buffer_read(&pid, contenido->buffer, sizeof(int));

                char* motivo_interrupcion = NULL;
                buffer_read(&motivo_interrupcion, contenido->buffer, sizeof(char*));

                free(motivo_interrupcion);
                eliminar_paquete(contenido);
                break;
            }

            case STDOUT: // STDOUT (Registro Dirección Lógica, Registro Tamaño)
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* dir_logica = NULL;
                buffer_read(&dir_logica, contenido->buffer, sizeof(char*));

                int tamanio;
                buffer_read(&tamanio, contenido->buffer, sizeof(int));

                ks_pasar_a_block(pcb);

                free(dir_logica);
                eliminar_paquete(contenido);
                break;
            }

            case STDIN: // STDIN (Registro Dirección Lógica, Registro Tamaño (DIRECCION LOGICA DONDE SE GUARDARÁ LO LEIDO))
            {
                t_paquete* contenido = recibir_paquete(fd);

                char* dir_logica = NULL;
                buffer_read(&dir_logica, contenido->buffer, sizeof(char*));

                int tamanio;
                buffer_read(&tamanio, contenido->buffer, sizeof(int));

                free(dir_logica);
                eliminar_paquete(contenido);
                break;
            }

            case SLEEP_IO: // SLEEP (TIEMPO EN MS, PID)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int tiempo_en_ms;
                buffer_read(&tiempo_en_ms, contenido->buffer, sizeof(int));

                int pid;
                buffer_read(&pid, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }


            case OK_DESALOJO_COMPACTACION_MEMORIA: 
            {

                break;
            }


            case OK_DESALOJO: // OK_DESALOJO (PID)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int pid;
                buffer_read(&pid, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }


            case OK_ALOJAR: // OK_ALOJAR (PID)
            {
                t_paquete* contenido = recibir_paquete(fd);

                int pid;
                buffer_read(&pid, contenido->buffer, sizeof(int));

                eliminar_paquete(contenido);
                break;
            }

            case -1:
            {
                log_warning(logger, "[CPU] se desconecto cpu");
                ejecutando = 0;
                break;
            }
            default:
            {
                log_warning(logger, "[CPU] operacion desconocida: %d", op_code);
                break;
            }
        }
    }
    close(fd);
    return NULL;
}

t_mutex_kernel* ks_buscar_mutex(char* nombre) {
    bool _es_mutex(void* ptr) {
        t_mutex_kernel* mutex = ptr;
        return strcmp(mutex->nombre, nombre) == 0;
    }

    pthread_mutex_lock(&mutex_mutexes_kernel);

    t_mutex_kernel* encontrado = list_find(mutexes_kernel, _es_mutex);

    pthread_mutex_unlock(&mutex_mutexes_kernel);

    return encontrado;
}

