#include "../include/finalizar_cpu.h"

void destruir_semaforos(){
	sem_destroy(&sem_fetch);
	sem_destroy(&sem_decode);
	sem_destroy(&sem_execute);
	sem_destroy(&sem_resize);
	sem_destroy(&sem_val_leido);
	sem_destroy(&sem_val_escrito);
	sem_destroy(&sem_sol_marco);
	sem_destroy(&sem_rta_kernel);
	sem_destroy(&sem_inst);
	sem_destroy(&sem_nro_marco);

	pthread_mutex_destroy(&mutex_interruptFlag);
	pthread_mutex_destroy(&mutex_manejo_contexto);
	pthread_mutex_destroy(&mutex_ejecute_wait_y_no_hay_instancias);

	pthread_mutex_destroy(&mutex_interrupt_pid);
	pthread_mutex_destroy(&mutex_interrupt_verificador);
	pthread_mutex_destroy(&mutex_interrupt_motivo);
	pthread_mutex_destroy(&mutex_desalojar);
	pthread_mutex_destroy(&mutex_fallo_algoritmo);
	pthread_mutex_destroy(&mutex_instruccion);
	pthread_mutex_destroy(&mutex_rta_m_resize);
	pthread_mutex_destroy(&mutex_rta_kernel);
	pthread_mutex_destroy(&mutex_flag_exit);
	pthread_mutex_destroy(&mutex_valor_leido_string);
	pthread_mutex_destroy(&mutex_valor_leido_uint32);
	pthread_mutex_destroy(&mutex_valor_leido_uint8);
	pthread_mutex_destroy(&mutex_valor_escrito);
	pthread_mutex_destroy(&mutex_op_autorizada);
	pthread_mutex_destroy(&mutex_instruccion_split);
	pthread_mutex_destroy(&mutex_TLB);
	pthread_mutex_destroy(&mutex_TLB_indices);
	pthread_mutex_destroy(&mutex_nro_marco);
	pthread_mutex_destroy(&mutex_tipo_desalojo);
	pthread_mutex_destroy(&mutex_nombre_instruccion_enum);

}

void finalizar_cpu(){
	destruir_semaforos();

	while(string_array_is_empty(op_autorizada)){
		free(string_array_pop(op_autorizada));
	}
	if(op_autorizada != NULL)
	{
		free(op_autorizada);
		op_autorizada = NULL;
	}

	if(un_contexto->r_cpu != NULL)
	{
		free(un_contexto->r_cpu);
		un_contexto->r_cpu = NULL;
	}

	if(un_contexto != NULL)
	{
		free(un_contexto);
		un_contexto = NULL;
	}

	log_destroy(cpu_log_debug);
	log_destroy(cpu_log_obligatorio);

	config_destroy(cpu_config);

	list_destroy_and_destroy_elements(TLB, free);
	list_destroy_and_destroy_elements(TLB_indices, free);

	// if(valor_leido_string != NULL){

	// 	free(valor_leido_string);
	// 	valor_leido_string = NULL;
	// }

	if(valor_escrito != NULL){

		free(valor_escrito);
		valor_escrito = NULL;
	}	

	if(instruccion != NULL){

		free(instruccion);
		instruccion = NULL;
	}
	
	if(rta_m_resize != NULL){

		free(rta_m_resize);
		rta_m_resize = NULL;
	}

	if(rta_kernel != NULL){

		free(rta_kernel);
		rta_kernel = NULL;
	}

	if(interrupt_motivo != NULL){

		free(interrupt_motivo);
		interrupt_motivo = NULL;
	}

	if(instruccion_split != NULL){
		string_array_destroy(instruccion_split);
		instruccion_split = NULL;
	}
		
	liberar_conexion(fd_kernel_dispatch);
	liberar_conexion(fd_kernel_interrupt);
	liberar_conexion(fd_memoria);
}
