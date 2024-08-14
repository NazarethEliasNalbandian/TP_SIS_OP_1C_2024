#include "../include/kernel_cpu_interrupt.h"
#include "../include/servicios_kernel.h"
#include "../include/pcb.h"
#include "../include/planificador_largo_plazo.h"
#include "../include/planificador_corto_plazo.h"

void _gestionar_interrupt_consola(){
	while(1){
		sem_wait(&sem_enviar_interrupcion_consola);

		pthread_mutex_lock(&mutex_lista_exec);
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_CPU_KERNEL);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->verificador);

		if(pcb_execute->flag_finalizar_proceso){
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_log_debug, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", pcb_execute->pid, pcb_execute->verificador);
		}
		else
			eliminar_paquete(un_paquete);
	}
}

void _gestionar_interrupt_quantum(){
	while(1){
		sem_wait(&sem_enviar_interrupcion_quantum);

		pthread_mutex_lock(&mutex_lista_exec);
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_CPU_KERNEL);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->verificador);

		if(pcb_execute->flag_finalizar_proceso){
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_log_debug, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", pcb_execute->pid, pcb_execute->verificador);
		}
		else{
			cargar_string_al_super_paquete(un_paquete, "FIN_QUANTUM");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
		}
	}
}

void _gestionar_interrupt_prioridad(){
	while(1){
		sem_wait(&sem_enviar_interrupcion_prioridad);

		pthread_mutex_lock(&mutex_lista_exec);
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		pthread_mutex_unlock(&mutex_lista_exec);

		t_paquete* un_paquete = crear_super_paquete(FORZAR_DESALOJO_CPU_KERNEL);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->pid);
		cargar_int_al_super_paquete(un_paquete, pcb_execute->verificador);

		if(pcb_execute->flag_finalizar_proceso){
			cargar_string_al_super_paquete(un_paquete, "DESALOJO_POR_CONSOLA");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);
			log_info(kernel_log_debug, "Send -> CPU: FORZAR_DESALOJO_KC <PID: %d>[T:%d]", pcb_execute->pid, pcb_execute->verificador);
		}
		else{
			cargar_string_al_super_paquete(un_paquete, "PRIORIDAD");
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);

		}
	}
}


void _desalojar_proceso(t_pcb* un_pcb){

	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que esa PCB siga en la lista
	if(list_remove_element(lista_execute, un_pcb)){
		liberar_recursos_pcb(un_pcb);
		avisar_a_memoria_para_liberar_estructuras(un_pcb);
		sem_wait(&sem_estructura_liberada);
		transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

		log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: INTERRUPTED_BY_USER", un_pcb->pid);
		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: INTERRUPTED_BY_USER", un_pcb->pid);
	}else{
		log_error(kernel_log_debug, "PCB no encontrada en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	//-----------------------
	sem_post(&sem_procesos_en_memoria);
	//-----------------------

	//pthread_mutex_lock(&mutex_flag_proceso_desalojado);
	un_pcb->flag_proceso_desalojado = false;
	//pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

	un_pcb->flag_finalizar_proceso = false;

	pcp_planificar_corto_plazo();
}

void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb, char* motivo_desalojo){
	
	log_info(kernel_log_debug, "REUBICACION DE PCB %d DE EXEC A READY", un_pcb->pid);

	pthread_mutex_lock(&mutex_lista_exec);
	pthread_mutex_lock(&mutex_lista_ready);
    pthread_mutex_lock(&mutex_lista_ready_prioridad);

	if(!list_remove_element(lista_execute, un_pcb)){
		log_error(kernel_log_debug ,"PCB_%d No esta en EXECUTE", un_pcb->pid);
		exit(EXIT_FAILURE);
	}

	cambiar_estado(un_pcb, READY);
	list_add(lista_ready, un_pcb);

	log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", un_pcb -> pid, estado_to_string(EXEC), estado_to_string(un_pcb->estado));

	char* pids_en_ready = lista_pids_en_ready();
	log_info(kernel_log_obligatorio, "Cola Ready / Ready Prioridad %s: %s", algoritmo_to_string(ALGORITMO_PLANIFICACION), pids_en_ready);

	if(pids_en_ready != NULL){

		free(pids_en_ready);
		pids_en_ready = NULL;
	}

	if(strcmp(motivo_desalojo, "FIN_QUANTUM") == 0)
	{
		log_info(kernel_log_obligatorio, "PID: %d - Desalojado por fin de Quantum", un_pcb->pid);
	}

    pthread_mutex_unlock(&mutex_lista_ready_prioridad);
	pthread_mutex_unlock(&mutex_lista_ready);
	pthread_mutex_unlock(&mutex_lista_exec);

	//pthread_mutex_lock(&mutex_flag_proceso_desalojado);
	un_pcb->flag_proceso_desalojado = false;
	//pthread_mutex_unlock(&mutex_flag_proceso_desalojado);

	pcp_planificar_corto_plazo();
}


void _atender_motivo_desalojo(char* motivo_desalojo, t_pcb* un_pcb){
	pausador();

	if(strcmp(motivo_desalojo, "DESALOJO_POR_CONSOLA") == 0){

        // pthread_mutex_lock(&mutex_flag_exit);
        // flag_cancelar_quantum = true;
        // pthread_mutex_unlock(&mutex_flag_exit);

		_desalojar_proceso(un_pcb);

	}else if(strcmp(motivo_desalojo, "FIN_QUANTUM") == 0 || strcmp(motivo_desalojo, "PRIORIDAD") == 0){

		if(un_pcb->flag_finalizar_proceso){
			// Esto sirve para darle prioridad al desalojo por consola
            // pthread_mutex_lock(&mutex_flag_exit);
            // flag_cancelar_quantum = true;
            // pthread_mutex_unlock(&mutex_flag_exit);

			ejecutar_en_un_hilo_nuevo_detach((void*)_desalojar_proceso, un_pcb);
			log_info(kernel_log_debug,"FLAG_EXIT %d", un_pcb->flag_finalizar_proceso);
		}else{
			_reubicar_pcb_de_execute_a_ready(un_pcb, motivo_desalojo);
		}
	}
}
