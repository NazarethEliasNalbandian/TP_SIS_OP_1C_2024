#include "../include/memoria_kernel.h"
#include "../include/proceso.h"

void atender_memoria_kernel(){

	bool control_key = 1;
	while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
			case ESTRUCTURA_INICIADA_KERNEL_MEMORIA://[char* path][int pid]
				unBuffer = recibir_paquete(fd_kernel);
				iniciar_estructura_para_un_proceso_nuevo(unBuffer);
				break;
			case ESTRUCTURA_LIBERADA_KERNEL_MEMORIA: //[int pid]
				unBuffer = recibir_paquete(fd_kernel);
				eliminar_proceso_y_liberar_estructuras(unBuffer);
				break;
			case -1:
				log_error(memoria_log_debug, "DESCONEXION DE KERNEL");
				control_key = 0;
				sem_post(&sem_finalizo_kernel);
				break;
			default:
				log_warning(memoria_log_debug,"OPERACION DESCONOCIDA DE KERNEL");
				sem_post(&sem_finalizo_kernel);
				break;
		}
	}
}

void iniciar_estructura_para_un_proceso_nuevo(t_buffer* un_Buffer){
	char* path = recibir_string_del_buffer(un_Buffer);
	int pid = recibir_int_del_buffer(un_Buffer);

	destruir_buffer(un_Buffer);
	
	t_proceso* un_proceso = crear_proceso(pid, path);

	pthread_mutex_lock(&mutex_lst_procss_recibidos);
	list_add(list_procss_recibidos, un_proceso);
	pthread_mutex_unlock(&mutex_lst_procss_recibidos);

	logg_crear_tabla_de_paginas(pid, cantidad_paginas_de_un_proceso(un_proceso));

	responder_a_kernel_confirmacion_del_proceso_creado();

	if(path != NULL){
		free(path);
		path = NULL;
	}
}

void eliminar_proceso_y_liberar_estructuras(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	t_proceso* un_proceso = obtener_proceso_por_id(pid);

	destruir_buffer(unBuffer);

	pthread_mutex_lock(&mutex_lst_procss_recibidos);

	if(list_remove_element(list_procss_recibidos, un_proceso)){
		if(un_proceso != NULL){

			eliminar_proceso(un_proceso);
			un_proceso = NULL;
		}
		log_info(memoria_log_debug, "DELETE <PID:%d>", pid);
	}
	else{
		log_error(memoria_log_debug, "Proceso no encontrado en la lista de procesos para ser eliminados");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&mutex_lst_procss_recibidos);
	
	enviar_a_kernel_rpta_de_eliminacion_de_proceso(pid);
}

// ENVÍOS A KERNEL

void responder_a_kernel_confirmacion_del_proceso_creado(){
	retardo_respuesta();
	t_paquete* un_paquete = crear_paquete(ESTRUCTURA_INICIADA_KERNEL_MEMORIA);
	cargar_string_al_paquete(un_paquete, "Proceso creado");
	enviar_paquete(un_paquete, fd_kernel);
	eliminar_paquete(un_paquete);
}

void enviar_a_kernel_rpta_de_eliminacion_de_proceso(){
	retardo_respuesta();
	t_paquete* un_paquete = crear_paquete(ESTRUCTURA_LIBERADA_KERNEL_MEMORIA);
	cargar_string_al_paquete(un_paquete, "Estructura liberada");
	enviar_paquete(un_paquete, fd_kernel);
	eliminar_paquete(un_paquete);
}