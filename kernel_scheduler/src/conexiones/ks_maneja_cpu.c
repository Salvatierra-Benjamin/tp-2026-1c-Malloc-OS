#include "ks_maneja_cpu.h"

#include "../main/ks_main.h"
#include "../logs/ks_logs.h"

void* ks_maneja_cpu(void* arg) {
    int fd = *(int*)arg;
    free(arg);

    log_info(ks_logger, "Entra en ks_maneja_cpu (fd=%d)", fd);
    // // Recibir CPU_IDENTIFICACION
    // int cpu_id = recibir_operacion(fd);
    // if(cpu_id != CPU_IDENTIFICACION) { 
    //     close(fd); 
    //     return; 
    // }

    // t_paquete* paquete = recibir_paquete(fd);
    // paquete->buffer->offset = 0;
    // uint32_t wid = read_u32_from_pkg(paquete);
    // eliminar_paquete(paquete);

    // // Registrar cpu
    // t_cpu* cpu = malloc(sizeof(t_cpu));

    // // Cada cpu tiene su hilo para recibir mensajes
    // for(;;) {
    //     int op = recibir_operacion(fd);
    //     if(op <= 0){
    //         // desconexion de cpu

    //         // uint32_t error_cpu = cpu->running_qid==0xFFFFFFFF ? 0xFFFFFFFF : w->running_qid;
    //         // log_worker_desconectado(cpu->id, error_cpu);

    //         // if(error_cpu != 0xFFFFFFFF) {
    //         //     // desconectar cp
    //         // }
    //         // // borrar cpu de lista

    //         close(fd);
    //         return;
    //     }

    //     t_paquete* paquete = recibir_paquete(fd);
    //     paquete->buffer->offset = 0;

    //     // switch(op) {
    //     //     case  {

    //     //     }

    //     //     default:
    //     //         break;
    //     // }

    //     eliminar_paquete(paquete);
    // }

    return NULL;
}
