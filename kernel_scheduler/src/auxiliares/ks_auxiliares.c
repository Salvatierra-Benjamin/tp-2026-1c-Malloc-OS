#include "ks_auxiliares.h"

#include "../main/ks_main.h"
void liberar_proceso(void* proc) {
    t_pcb* proceso = (t_pcb*) proc;
    free(proceso->path); // Liberar la memoria del path
    free(proceso); // Liberar la memoria del PCB
}

void liberar_proceso_bsod(void* proc) {
    t_pcb* proceso = (t_pcb*) proc;
    log_fin_proceso(proceso, "Blue Screen of Death (BSOD)");
    free(proceso->path); // Liberar la memoria del path
    free(proceso); // Liberar la memoria del PCB
}

void matar_procesos_ready() {
    for (int i = 0; i < cantidad_algoritmos; i++) {
        queue_destroy_and_destroy_elements(array_queues[i].queue, liberar_proceso_bsod);
        queue_destroy_and_destroy_elements(array_queues[i].queue_block, liberar_proceso_bsod);
        queue_destroy_and_destroy_elements(array_queues[i].queue_susp_block, liberar_proceso_bsod);
        queue_destroy_and_destroy_elements(array_queues[i].queue_susp_ready, liberar_proceso_bsod);
    }
}

void liberar_listas() {
    pthread_mutex_lock(&mutex_cpus);
    list_destroy_and_destroy_elements(cpus, liberar_proceso);
    pthread_mutex_unlock(&mutex_cpus);

    pthread_mutex_lock(&mutex_ios);
    list_destroy_and_destroy_elements(ios, liberar_proceso);
    pthread_mutex_unlock(&mutex_ios);
    // list_destroy_and_destroy_elements(procesos, liberar_proceso);

    pthread_mutex_lock(&mutex_exec);
    list_destroy_and_destroy_elements(procs_exec, liberar_proceso);
    pthread_mutex_unlock(&mutex_exec);
    // pthread_mutex_lock(&mutex_block);
    // list_destroy_and_destroy_elements(procs_block, liberar_proceso);
    // pthread_mutex_unlock(&mutex_block);
    // pthread_mutex_lock(&mutex_susp_block);
    // list_destroy_and_destroy_elements(procs_susp_block, liberar_proceso);
    // pthread_mutex_unlock(&mutex_susp_block);
    // pthread_mutex_lock(&mutex_susp_ready);
    // list_destroy_and_destroy_elements(procs_susp_ready, liberar_proceso);
    // pthread_mutex_unlock(&mutex_susp_ready);
    // pthread_mutex_lock(&mutex_exit);
    list_destroy_and_destroy_elements(procs_exit, liberar_proceso);
    pthread_mutex_unlock(&mutex_exit);
}

void liberar_mutex() {
    pthread_mutex_destroy(&mutex_cpus);
    pthread_mutex_destroy(&mutex_ios);
    pthread_mutex_destroy(&mutex_exec);
    // pthread_mutex_destroy(&mutex_block);
    // pthread_mutex_destroy(&mutex_susp_block);
    // pthread_mutex_destroy(&mutex_susp_ready);
    pthread_mutex_destroy(&mutex_exit);
    pthread_mutex_destroy(&mutex_array_queues);

    for (int i = 0; i < cantidad_algoritmos; i++) {
        pthread_mutex_destroy(&array_queues[i].mutex_ready);
        pthread_mutex_destroy(&array_queues[i].mutex_block);
        pthread_mutex_destroy(&array_queues[i].mutex_susp_block);
        pthread_mutex_destroy(&array_queues[i].mutex_susp_ready);   
    }
}

bool finalizar_proceso(int pid) {
    t_pcb* pcb = buscar_pcb_por_pid(pid);
    if (pcb == NULL) {
        return false; // No se encontró el proceso con el PID dado
    }
    pcb->estado = EXIT; // Cambia el estado del proceso a "EXIT"
    list_add(procs_exit, pcb); // Agrega el proceso a la lista de procesos en estado EXIT

    return true;
} 

t_pcb* crear_proceso(int pid, char* path, int prioridad) {
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    if (nuevo_pcb == NULL) {
        return NULL; // Manejar error de asignación de memoria
    }
    nuevo_pcb->pid = pid;
    nuevo_pcb->path = strdup(path); // Asegúrate de liberar esta memoria cuando ya no se necesite
    nuevo_pcb->prioridad = prioridad;
    nuevo_pcb->cpu_fd = -1; // Inicialmente no asignado a ningún CPU
    nuevo_pcb->io_fd = -1;  // Inicialmente no asignado a ningún IO
    nuevo_pcb->estado = READY; // Estado inicial del proceso es "READY" (saltea NEW)

    queue_push(array_queues[prioridad].queue, nuevo_pcb); // Agrega el proceso a la cola correspondiente según su prioridad
    return nuevo_pcb;
}


void ks_liberar_array_queues(void) {
	if (array_queues) {
		for (int i = 0; i < cantidad_algoritmos; i++) {
			queue_destroy(array_queues[i].queue);
		}

		free(array_queues);
	}
}
t_person *find_by_name_contains(t_list *people, char *substring) {
	bool _name_contains(void *ptr) {
		t_person *person = (t_person *)ptr;
		return string_contains(person->name, substring);
	}
	return list_find(people, _name_contains);
}
t_pcb *buscar_pcb_por_pid(int pid) {
	for (int i = 0; i < cantidad_algoritmos; i++) {

		bool _pcb_tiene_pid(void *ptr) {
			t_pcb *pcb = (t_pcb *)ptr;
			return pcb->id == pid;
		}

		t_pcb *encontrado = list_find(array_queues[i].queue, _pcb_tiene_pid);

		if (encontrado != NULL) {
			return encontrado;
		}
	}
	return NULL; // No se encontró el PCB con el PID dado
}

