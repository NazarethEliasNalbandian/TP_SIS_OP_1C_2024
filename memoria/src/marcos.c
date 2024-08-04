#include "../include/marcos.h"

t_marco* crear_marco(int base, bool libre, int index){
	t_marco *nuevo_marco = malloc(sizeof(t_marco));
	nuevo_marco->nro_marco = index;
	nuevo_marco->base = base;
	nuevo_marco->libre = libre;
	nuevo_marco->info_new = NULL;
	nuevo_marco->info_old = NULL;

	return nuevo_marco;
}

void liberar_marco(t_marco* un_marco){
	un_marco->libre = true;
	if(un_marco->info_new != NULL){
		free(un_marco->info_new);
		un_marco->info_new = NULL;
	}
	if(un_marco->info_old != NULL){
		free(un_marco->info_old);
		un_marco->info_old = NULL;
	}
}

t_marco* obtener_marco_por_nro_marco(int nro_marco){
	t_marco* un_marco;
	pthread_mutex_lock(&mutex_lst_marco);
	un_marco = list_get(lst_marco, nro_marco);
	pthread_mutex_unlock(&mutex_lst_marco);

	return un_marco;
}

t_marco* obtener_un_marco_libre_de_la_lista_de_marcos(){
	t_marco* un_marco = NULL;

	bool _marco_libre(t_marco* un_marco){
		return un_marco->libre;
	};
	pthread_mutex_lock(&mutex_lst_marco);
	un_marco = list_find(lst_marco, (void*)_marco_libre);
	pthread_mutex_unlock(&mutex_lst_marco);

	return un_marco;
}

void destruir_list_marcos_y_todos_los_marcos(){
	void _destruir_un_marco(t_marco* un_marco){
		if(un_marco->info_new != NULL){
			free(un_marco->info_new);
			un_marco->info_new = NULL;
		}
		if(un_marco->info_old != NULL){
			free(un_marco->info_old);
			un_marco->info_old = NULL;
		}
		if(un_marco != NULL){

			free(un_marco);
			un_marco = NULL;
		}
	};
	pthread_mutex_lock(&mutex_lst_marco);
	// list_destroy_and_destroy_elements(lst_marco, (void*)_destruir_un_marco);
	pthread_mutex_unlock(&mutex_lst_marco);
}

int cantidad_de_marcos_libres() {
    bool _marco_libre(t_marco* un_marco) {
        return un_marco->libre;
    };
	pthread_mutex_lock(&mutex_lst_marco);
    t_list* lista_filtrada = list_filter(lst_marco, (void*)_marco_libre);
    int cantidad = list_size(lista_filtrada);
    list_destroy(lista_filtrada);  
	pthread_mutex_unlock(&mutex_lst_marco);
    return cantidad;
}