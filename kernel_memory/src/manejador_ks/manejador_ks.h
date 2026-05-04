#ifndef MANEJADOR_KS
#define MANEJADOR_KS

#include <recursos_globales/recursos_globales.h>

void *manejador_ks(void *arg);
void creacion_proceso(int PID, char* PATH);
t_list *dividir_archivo_por_lineas(char *path);
char *armar_ruta(char *ruta_relativa);

#endif