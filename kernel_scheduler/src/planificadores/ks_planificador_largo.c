#include "ks_planificador_largo.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"
#include "../auxiliares/ks_auxiliares.h"

void* ks_planificador_largo_plazo(void* arg) {
    (void)arg;

    while (1) {

    }

    return NULL;
}

void ks_admitir_proceso(t_pcb* pcb) {
    (void)pcb;
}

void ks_finalizar_proceso_planificador(t_pcb* pcb) {
    (void)pcb;
}

bool ks_hay_espacio_memoria(void) {
    return true;
}