#ifndef RECURSO_H_
#define RECURSO_H_

#include "k_gestor.h"

t_recurso* buscar_recurso(char* recurso_solicitado);
void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recurso, pthread_mutex_t mutex_recurso);

#endif 