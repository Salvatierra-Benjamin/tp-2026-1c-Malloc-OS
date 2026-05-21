#include "ks_planificador_mediano.h"

#include "../main/ks_main.h"

#include "../logs/ks_logs.h"
#include "../auxiliares/ks_auxiliares.h"

void* ks_planificador_mediano_plazo(void* arg) {
    (void)arg;

    while (1) {

    }

    return NULL;
}

void ks_suspendar_proceso(t_pcb* pcb) {
    (void)pcb;
}

void ks_reanudar_proceso(t_pcb* pcb) {
    (void)pcb;
}

bool ks_supera_tiempo_suspension(t_pcb* pcb) {
    (void)pcb;
    return false;
}