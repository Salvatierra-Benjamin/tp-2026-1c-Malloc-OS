#include "mmu.h"
// IMPLEMENTACION MMU con parametros de entrada por referencia (para evitar confusiones con el paso por valor de bytes_validos y dir_fisica)
int MMU_TRADUCCION_FISICA(int dir_logica, uint32_t tam, int *dir_fisica, uint32_t *bytes_validos)
{
    uint32_t tam_max_segmentos = Get_tam_max_segmentos();
    uint32_t num_segmento = dir_logica / tam_max_segmentos;
    uint32_t desplazamiento = dir_logica % tam_max_segmentos;

    if (num_segmento >= contexto_ejecucion.tabla_segmentos.cantidad_segmentos)
    {
        log_error(cpu_logger, "Seg. Invalido");
        liberar_todo();

        exit(EXIT_FAILURE);
    }

    segmento_t seg = contexto_ejecucion.tabla_segmentos.segmento[num_segmento];
    if (desplazamiento >= seg.tam_seg)
    {
        log_error(cpu_logger, "Seg. fault en funcion MMU_TRADUCCION_FISICA()");

        uint8_t motivo = AVISO_DE_INTERRUPCION_SEGMENTATION_FAULT;
        t_paquete *paquete_ks = crear_paquete(DESALOJO_PROCESO);
        int pid = contexto_actual->PID;
        agregar_a_paquete(paquete_ks, &pid, sizeof(int));
        agregar_a_paquete(paquete_ks, &motivo, sizeof(uint8_t));
        enviar_paquete(paquete_ks, conexion_ks);
        eliminar_paquete(paquete_ks);
        liberar_todo();

        exit(EXIT_FAILURE);
    }

    uint32_t dispo = seg.tam_seg - desplazamiento;
    *bytes_validos = (tam < dispo) ? tam : dispo;
    *dir_fisica = seg.dir_base + desplazamiento;
    return 0;
}
typedef struct
{
    int id_stick;       // Identificador único
    int socket_fd;      // File descriptor de la conexión
    uint32_t size;      // Tamaño reportado por el stick
    uint32_t base_addr; // Dirección física global donde "empieza" este stick
} t_memory_stick;

// Retorna el MS correspondiente y guarda en 'local_offset' la dirección real a enviar
t_memory_stick *encontrar_memory_stick(t_list *lista_sticks, uint32_t global_addr, uint32_t *local_offset)
{
    for (int i = 0; i < list_size(lista_sticks); i++)
    {
        t_memory_stick *ms = list_get(lista_sticks, i);

        // Verificamos si la dirección global cae dentro del rango de este Memory Stick
        if (global_addr >= ms->base_addr && global_addr < (ms->base_addr + ms->size))
        {
            *local_offset = global_addr - ms->base_addr;
            return ms;
        }
    }
    return NULL; // Segmentation Fault: La dirección no existe en la memoria física
}

void escribir_en_memoria(t_list *lista_sticks_cpu, uint32_t dir_global_inicial, void *buffer, uint32_t tamanio_total)
{
    uint32_t bytes_escritos = 0;
    uint32_t dir_actual = dir_global_inicial;

    while (bytes_escritos < tamanio_total)
    {
        uint32_t offset_local = 0;
        t_memory_stick *ms_destino = encontrar_memory_stick(lista_sticks_cpu, dir_actual, &offset_local);

        if (ms_destino == NULL)
        {
            // Manejar error de Out of Memory / SEG_FAULT
            log_error(logger, "Intento de acceso fuera de los límites físicos.");
            break;
        }

        // Calcular cuántos bytes podemos escribir en este Memory Stick sin pasarnos de su límite
        uint32_t espacio_disponible_en_stick = ms_destino->size - offset_local;
        uint32_t bytes_restantes = tamanio_total - bytes_escritos;

        uint32_t bytes_a_escribir_ahora = (bytes_restantes < espacio_disponible_en_stick) ? bytes_restantes : espacio_disponible_en_stick;

        // Armamos el paquete y lo enviamos al socket específico del Memory Stick
        t_paquete *paquete = crear_paquete(OP_ESCRITURA);
        agregar_a_paquete(paquete, &offset_local, sizeof(uint32_t));
        agregar_a_paquete(paquete, buffer + bytes_escritos, bytes_a_escribir_ahora);

        enviar_paquete(paquete, ms_destino->socket_fd);
        eliminar_paquete(paquete);

        // Esperar la confirmación (OK) del Memory Stick antes de seguir
        int respuesta = recibir_operacion(ms_destino->socket_fd);
        if (respuesta != OP_OK)
        {
            log_error(logger, "Error al escribir en el MS %d", ms_destino->id_stick);
            break;
        }

        // Actualizamos contadores para la siguiente iteración (si la hay)
        bytes_escritos += bytes_a_escribir_ahora;
        dir_actual += bytes_a_escribir_ahora;
    }
}