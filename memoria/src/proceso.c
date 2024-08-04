#include "../include/proceso.h"


t_proceso* crear_proceso(int pid, char* path_instruc){
	t_proceso* proceso_nuevo = malloc(sizeof(t_proceso));
	proceso_nuevo->pid = pid;
	proceso_nuevo->pathInstrucciones = strdup(path_instruc);
	proceso_nuevo->instrucciones = NULL;
	proceso_nuevo->tabla_paginas = list_create();
	pthread_mutex_init(&(proceso_nuevo->mutex_TP), NULL);

	//Cargando instrucciones
	proceso_nuevo->instrucciones = leer_archivo_y_cargar_instrucciones(proceso_nuevo->pathInstrucciones);

	// free(path_instruc);
	return proceso_nuevo;
}

void eliminar_proceso(t_proceso* un_proceso){

	if(un_proceso->pathInstrucciones != NULL){

		free(un_proceso->pathInstrucciones);
		un_proceso->pathInstrucciones = NULL;
	}

	eliminar_lista_de_instrucciones(un_proceso->instrucciones);

	//Eliminar Tabla de Paginas - y liberar marcos correspondientes
	eliminar_tabla_de_paginas(un_proceso);

	pthread_mutex_destroy(&(un_proceso->mutex_TP));

	if(un_proceso != NULL){

		free(un_proceso);
		un_proceso = NULL;
	}
}

void eliminar_tabla_de_paginas(t_proceso* un_proceso){
	int cant_paginas = list_size(un_proceso->tabla_paginas);
	void _eliminar_pagina_y_liberar_marco(t_pagina* una_pagina){
		//Marcar como libre el marco correspondiente
		t_marco* un_marco = obtener_marco_por_nro_marco(una_pagina->nro_marco);
		liberar_marco(un_marco);
		
		if(una_pagina != NULL){

			free(una_pagina);
			una_pagina = NULL;
		}
	};
	list_destroy_and_destroy_elements(un_proceso->tabla_paginas, (void*)_eliminar_pagina_y_liberar_marco);

	//Log obligatorio destruccion de tabla de paginas
	logg_destruir_tabla_de_paginas(un_proceso->pid, cant_paginas);
}

int cantidad_paginas_de_un_proceso(t_proceso* un_proceso){
	return list_size(un_proceso->tabla_paginas);
}

void eliminar_paginas_a_un_proceso(t_proceso* un_proceso, int cantidad_paginas_a_eliminar){
	void _eliminar_pagina_y_liberar_marco(t_pagina* una_pagina){
		t_marco* un_marco = obtener_marco_por_nro_marco(una_pagina->nro_marco);
		liberar_marco(un_marco);
		if(una_pagina != NULL){

			free(una_pagina);
			una_pagina = NULL;
		}
	};
	int cant_paginas_totales = cantidad_paginas_de_un_proceso(un_proceso);

	for(int i=0;i<cantidad_paginas_a_eliminar;i++){

		list_remove_and_destroy_element(un_proceso->tabla_paginas,cant_paginas_totales-1-i, (void*)_eliminar_pagina_y_liberar_marco);
		log_info(memoria_log_debug, "SE ELIMINO LA PAGINA NRO:%d", cant_paginas_totales-1-i);
	}
}

void eliminar_lista_de_instrucciones(t_list* lista_instrucciones){
	
	void _destruir_instruccion(char* una_instruccion){
		if(una_instruccion != NULL){

			free(una_instruccion);
			una_instruccion = NULL;
		}
	};
	list_destroy_and_destroy_elements(lista_instrucciones, (void*)_destruir_instruccion);
}

t_proceso* obtener_proceso_por_id(int pid){
	bool _buscar_el_pid(t_proceso* proceso){
		return proceso->pid == pid;
	};
	
	pthread_mutex_lock(&mutex_lst_procss_recibidos);
	t_proceso* un_proceso = list_find(list_procss_recibidos, (void*)_buscar_el_pid);
	pthread_mutex_unlock(&mutex_lst_procss_recibidos);

	if(un_proceso == NULL){
		log_error(memoria_log_debug, "PID<%d> No encontrado en la lista de procesos", pid);
		exit(EXIT_FAILURE);
	}
	return un_proceso;
}

char* obtener_instruccion_por_indice(t_proceso* un_proceso, int indice_instruccion){
	char* instruccion_actual;
	if(indice_instruccion >= 0 && indice_instruccion < list_size(un_proceso->instrucciones)){
		instruccion_actual = list_get(un_proceso->instrucciones, indice_instruccion);
		return instruccion_actual;
	}
	else{
		log_error(memoria_log_debug, "PID<%d> - Nro de Instruccion <%d> NO VALIDA", un_proceso->pid, indice_instruccion);
		return NULL;
	}
}

int agregar_paginas_a_un_proceso(t_proceso* un_proceso, int cantidad_paginas){
	
	if(puedo_cargar_paginas_en_proceso(cantidad_paginas) == 1) {

		int cantidad_paginas_proceso = cantidad_paginas_de_un_proceso(un_proceso);
		for(int i=0; i<cantidad_paginas; i++){
			t_pagina* una_pagina = malloc(sizeof(t_pagina));
			una_pagina->nro_pagina = cantidad_paginas_proceso+i;

			t_marco* un_marco_libre = obtener_un_marco_libre_de_la_lista_de_marcos();
			if(un_marco_libre->info_old != NULL){
				free(un_marco_libre->info_old);
				un_marco_libre->info_old = NULL;
			}
			un_marco_libre->info_old = un_marco_libre->info_new;
			un_marco_libre->info_new = malloc(sizeof(frame_info));
			un_marco_libre->info_new->proceso = un_proceso;
			un_marco_libre->info_new->nro_pagina = una_pagina->nro_pagina;
			un_marco_libre->libre = false;

			una_pagina->nro_marco = un_marco_libre->nro_marco;
			log_info(memoria_log_debug, "SE ASIGNO EL  MARCO %d A LA PAGINA: %d",una_pagina->nro_marco, una_pagina->nro_pagina);
			list_add(un_proceso->tabla_paginas, una_pagina);
		}
		return 0;
	}
	else {
				
		enviar_OUT_OF_MEMORY();
		log_error(memoria_log_debug, "OUT OF MEMORY");
		return -1;
	}

}

bool puedo_cargar_paginas_en_proceso(int cantidad_paginas) {
	return cantidad_paginas <= cantidad_de_marcos_libres();
}

t_pagina* pag_obtener_pagina_completa(t_proceso* un_proceso, int nro_pagina){
	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pagina);
	return una_pagina;
}

