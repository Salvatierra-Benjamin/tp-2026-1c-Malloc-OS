#include "ks_planificador_corto.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"
#include "../auxiliares/ks_auxiliares.h"

void* ks_planificador_corto_plazo(void* arg) {
    (void)arg;

    while (1) {

    }

    return NULL;
}

void ks_planificar_fifo(void) {

}

void ks_planificar_rr(void) {

}

void ks_planificar_cmn(void) {

}

t_pcb* ks_obtener_siguiente_ready(void) {
    return NULL;
}

bool ks_hay_procesos_ready(void) {
    return false;
}

void ks_enviar_a_exec(t_pcb* pcb, int cpu_fd) {
    (void)pcb;
    (void)cpu_fd;
}