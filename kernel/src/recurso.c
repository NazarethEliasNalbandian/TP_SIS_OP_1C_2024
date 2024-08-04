#include "../include/recurso.h"

t_recurso* buscar_recurso(char* recurso_solicitado){
// Capaz es necesario utilziar mutex de la lista de recursos
	// pthread_mutex_lock(&mutex_lista_recursos);
	for(int i = 0; i<list_size(lista_recursos); i++){
		t_recurso* r_buscado = list_get(lista_recursos,i);
		if(strcmp(recurso_solicitado, r_buscado->recurso_name) == 0){
			return r_buscado;
		}
	}
	// pthread_mutex_unlock(&mutex_lista_recursos);
	return NULL;
}

void agregar_pcb_a_lista_recurso(t_pcb* pcb, t_list* lista_recurso, pthread_mutex_t mutex_recurso){
	pthread_mutex_lock(&mutex_lista_recursos);
	pthread_mutex_lock(&mutex_recurso);
	list_add(lista_recurso, pcb);
	pthread_mutex_unlock(&mutex_recurso);
	pthread_mutex_unlock(&mutex_lista_recursos);
}
