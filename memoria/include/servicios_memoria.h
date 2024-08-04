#ifndef SERVICIOS_MEMORIA_H_
#define SERVICIOS_MEMORIA_H_

#include "m_gestor.h"

void retardo_respuesta();
void logg_crear_tabla_de_paginas(int pid, int cant_paginas);
void logg_destruir_tabla_de_paginas(int pid, int cant_paginas);
void logg_acceso_a_tabla_de_paginas(int pid, int nro_pagina, int nro_marco);
void logg_ampliacion_proceso(int pid, int tamanio_actual, int tamiano_a_ampliar);
void logg_reduccion_proceso(int pid, int tamanio_actual, int tamiano_a_reducir);
void logg_acceso_a_espacio_de_usuario(int pid, char* accion, int dir_fisica, size_t tamanio);

#endif /* SERVICIOS_MEMORIA_H_ */