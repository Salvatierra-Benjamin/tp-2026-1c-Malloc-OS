#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <../../utils/src/utils/protocolos.h>
#include "ks_main.h"

void ks_maneja_cpu(int fd) {
    // Recibir CPU_IDENTIFICACION
    int cpu_id = recibir_operacion(fd);
    if(cpu_id != CPU_IDENTIFICACION) { 
        close(fd); 
        return; 
    }

    t_paquete* paquete = recibir_paquete(fd);
    paquete->buffer->offset = 0;
    uint32_t wid = read_u32_from_pkg(paquete);
    eliminar_paquete(paquete);

    // Registrar cpu
    cpu_t* cpu = malloc(sizeof(cpu_t));

    // Cada cpu tiene su hilo para recibir mensajes
    for(;;) {
        int op = recibir_operacion(fd);
        if(op <= 0){
            // desconexion de cpu

            uint32_t error_cpu = cpu->running_qid==0xFFFFFFFF ? 0xFFFFFFFF : w->running_qid;
            log_worker_desconectado(cpu->id, error_cpu);

            if(error_cpu != 0xFFFFFFFF) {
                // desconectar cp
            }
            // borrar cpu de lista

            close(fd);
            return;
        }

        t_paquete* paquete = recibir_paquete(fd);
        paquete->buffer->offset = 0;

        // switch(op) {
        //     case  {

        //     }

        //     default:
        //         break;
        // }

        eliminar_paquete(pk);
    }
}
