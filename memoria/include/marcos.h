#ifndef MARCOS_H_
#define MARCOS_H_

#include "m_gestor.h"
#include "servicios_memoria.h"

t_marco* crear_marco(int base, bool libre, int index);
void liberar_marco(t_marco* un_marco);
t_marco* obtener_marco_por_nro_marco(int nro_marco);
t_marco* obtener_un_marco_libre_de_la_lista_de_marcos();
void destruir_list_marcos_y_todos_los_marcos();
int cantidad_de_marcos_libres();

//---------------------


#endif /* MARCOS_H_ */
