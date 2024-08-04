#include "../include/cpu_kernel_interrupt.h"

void atender_cpu_kernel_interrupt(){
    bool control_key=1;
	
    while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel_interrupt,cpu_log_debug);
			break;
		case FORZAR_DESALOJO_CPU_KERNEL:
			unBuffer =  __recibiendo_super_paquete(fd_kernel_interrupt); 
			manejar_interrupcion(unBuffer);
			break;
		case -1:
			log_error(cpu_log_debug, "Desconexión de Kernel Interrupt");
            control_key=0;
			break;
		default:
			log_warning(cpu_log_debug,"Operacion desconocida de Kernel Interrupt.");
			break;
		}
	}
}

void manejar_interrupcion(t_buffer * unBuffer){
	//pthread_mutex_lock(&mutex_interruptFlag);
	interrupt_pid = __recibir_int_del_buffer(unBuffer);

	if(interrupt_pid == un_contexto->pID){
		
		interrupt_verificador = __recibir_int_del_buffer(unBuffer);
		
		if(interrupt_motivo != NULL){
			free(interrupt_motivo);
			interrupt_motivo = NULL;
		}
		
		interrupt_motivo = __recibir_string_del_buffer(unBuffer);
		destruir_buffer(unBuffer);
		
		log_warning(cpu_log_debug, "INTERRUPCION RECIBIDA: <PID:%d>[V:%d][%s]",interrupt_pid,interrupt_verificador,interrupt_motivo);

		interrupt_flag = 1;
	}
	else{
		log_warning(cpu_log_debug, "INTERRUPCION RECHAZADA");
		destruir_buffer(unBuffer);
	}
	//pthread_mutex_unlock(&mutex_interruptFlag);
}