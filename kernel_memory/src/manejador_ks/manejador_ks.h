#ifndef MANEJADOR_KS
#define MANEJADOR_KS

#include <recursos_globales/recursos_globales.h>

void *manejador_ks(void *arg);

// envíos a KS con mutex y chequeo de conexión (socket == -1 => descarta y loguea)
void enviar_operacion_a_ks(op_code operacion);
void enviar_paquete_a_ks(t_paquete *paquete);
void creacion_proceso(int PID, char* PATH, int prioridad);
t_list *dividir_archivo_por_lineas(char *path);
char *armar_ruta(char *ruta_relativa);
t_list *convertir_lineas_en_instrucciones(t_list *lineas);
t_instruccion *linea_a_instruccion(char *linea);
char *palabra_reservada_a_string(t_palabra_reservada comando);
t_palabra_reservada string_a_palabra_reservada(char *str);
t_contexto_ejecucion *crear_contexto_ejecucion(int pid);
void finalizar_proceso(int pid);

// segmentos
uint32_t crear_segmento(int pid, int sid, uint32_t tamanio, bool *necesita_compactacion);
bool eliminar_segmento(int pid, int sid);
void responder_mem_alloc_a_ks(int pid, uint32_t base);

// compactación
// Flujo: alloc fragmentado -> iniciar_compactacion (guarda el alloc pendiente y manda
// DESALOJO_POR_COMPACTACION a KS) -> KS desaloja CPUs y responde
// CONFIRMACION_DESALOJO_COMPACTACION -> compactar() -> reintento del alloc ->
// respuesta a KS + FIN_COMPACTACION. Todo corre en el hilo de manejador_ks.
void iniciar_compactacion(int pid, int sid, uint32_t tamanio);
void compactar(void);

// huecos
void mergear_huecos(); // asume mutex_lista_huecos tomado por el caller
void agregar_hueco_por_stick(t_memory_stick *stick);

#endif