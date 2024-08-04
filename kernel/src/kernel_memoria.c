#include "../include/kernel_memoria.h"

void atender_kernel_memoria(){
	bool control_key = 1;
	while (control_key) {
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer = NULL;
		switch (cod_op) {
			case ESTRUCTURA_INICIADA_KERNEL_MEMORIA:
				unBuffer = __recibiendo_super_paquete(fd_memoria);
				recibir_confirmacion_de_memoria(unBuffer);
				sem_post(&sem_estructura_iniciada);
				break;
			case ESTRUCTURA_LIBERADA_KERNEL_MEMORIA:
				unBuffer = __recibiendo_super_paquete(fd_memoria);
				recibir_confirmacion_de_memoria(unBuffer);
				sem_post(&sem_estructura_liberada);
				break;
			case -1:
				log_error(kernel_log_debug, "DESCONEXION DE MEMORIA");
				control_key = 0;
				break;
			default:
				log_warning(kernel_log_debug,"OPERACION DESCONOCIDA DE MEMORIA");
				break;
		}
	}
}

void recibir_confirmacion_de_memoria(t_buffer* unBuffer){
	char* recibir_mensaje = __recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);
	printf("MENSAJE DE MEMORIA: %s\n", recibir_mensaje);

	if(recibir_mensaje != NULL){
		free(recibir_mensaje);
		recibir_mensaje = NULL;
	}
}
