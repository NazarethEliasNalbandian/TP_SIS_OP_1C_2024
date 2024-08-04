#ifndef PROCESO_H_
#define PROCESO_H_

#include "m_gestor.h"
#include "marcos.h"
#include "servicios_memoria.h"
#include "memoria_cpu.h"
#include "memoria_kernel.h"
#include "memoria_entradasalida.h"
#include "espacio_usuario.h"

t_proceso* crear_proceso(int pid, char* path_instruc);
void eliminar_proceso(t_proceso* un_proceso);
void eliminar_tabla_de_paginas(t_proceso* un_proceso);
int cantidad_paginas_de_un_proceso(t_proceso* un_proceso);
void eliminar_paginas_a_un_proceso(t_proceso* un_proceso, int cantidad_paginas_a_eliminar);
void eliminar_lista_de_instrucciones(t_list* lista_instrucciones);
t_proceso* obtener_proceso_por_id(int pid);
char* obtener_instruccion_por_indice(t_proceso* un_proceso, int indice_instruccion);
int agregar_paginas_a_un_proceso(t_proceso* un_proceso, int cantidad_paginas);
bool puedo_cargar_paginas_en_proceso(int cantidad_paginas);
t_pagina* pag_obtener_pagina_completa(t_proceso* un_proceso, int nro_pagina);
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);

#endif /* PROCESO_H_ */