#ifndef MANEJADOR_KS
#define MANEJADOR_KS

#include <recursos_globales/recursos_globales.h>

void *manejador_ks(void *arg);
void creacion_proceso(int PID, char* PATH, int prioridad);
t_list *dividir_archivo_por_lineas(char *path);
char *armar_ruta(char *ruta_relativa);
t_list *convertir_lineas_en_instrucciones(t_list *lineas);
t_instruccion *linea_a_instruccion(char *linea);
char *palabra_reservada_a_string(t_palabra_reservada comando);
t_palabra_reservada string_a_palabra_reservada(char *str);
t_contexto_ejecucion *crear_contexto_ejecucion(int pid);

#endif