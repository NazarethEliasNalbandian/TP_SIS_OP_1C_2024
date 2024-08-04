#include "../include/planificador_corto_plazo.h"
#include "../include/servicios_kernel.h"
#include "../include/pcb.h"

void _programar_interrupcion_por_quantum(t_pcb* un_pcb){
	int verificador_referencia = un_pcb->verificador;
	usleep(un_pcb->quantum*1000);

	/*Esta comprobacion de ticket es en caso de que la PCB haya salido de CPU,
	 * Puesto en READY y por casulaidades de la vida haya vuelto a la CPU
	 * Y al despertar este hilo, primero verifique que la PCB objetivo, no haya
	 * salido de la CPU, y esto lo resolvemos con el ticket.
	 * Porque si salio la misma PCB y volvio a entrar, significa que el proceso tiene
	 * nuevo ticket*/
	
	pthread_mutex_lock(&mutex_verificador);
	if(verificador_referencia == var_verificador){
		if(!un_pcb->flag_cancelar_quantum){


			sem_post(&sem_enviar_interrupcion_quantum);
			log_info(kernel_log_debug, "FIN DE QUANTUM DE %d", un_pcb->pid);
		}
	}
	pthread_mutex_unlock(&mutex_verificador);
}

void _atender_RR_FIFO(){
	//Verificar que la lista de EXECUTE esté vacía

	pthread_mutex_lock(&mutex_lista_exec);
	if(list_is_empty(lista_execute)){
		t_pcb* un_pcb = NULL;

		//Verificar que haya elementos en la lista de READY
		pthread_mutex_lock(&mutex_lista_ready);
		if(!list_is_empty(lista_ready)){
			un_pcb = list_remove(lista_ready, 0);
		}
		pthread_mutex_unlock(&mutex_lista_ready);

		if(un_pcb != NULL){
			list_add(lista_execute, un_pcb);
			cambiar_estado(un_pcb, EXEC);
			log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: READY - Estado Actual: EXEC", un_pcb->pid);
			un_pcb->verificador= generar_verificador();

			_enviar_pcb_CPU_dispatch(un_pcb);
			log_info(kernel_log_debug,"ENVIE PCB A CPU");

			if(ALGORITMO_PLANIFICACION == ROUNDROBIN){
				un_pcb->flag_cancelar_quantum = false;
				ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, un_pcb);
			}
		}else{
			log_warning(kernel_log_debug, "Lista de READY vacía");
		}
	}
	pthread_mutex_unlock(&mutex_lista_exec);
}

void _atender_VRR(){

	// SI UN PROCESO

	// MANDO EL PROCESO A EJECUTAR, ARRANCO EL TIMER. HECHO
	// CUANDO LLEGA UNA SYSCALL (EJECUCION DE IO O DE WAIT (SIN TENER RECURSOS DISPONIBLES)), FRENO EL TIMER. HECHO
	// CUANDO VUELVE DE SU BLOQUEO, CALCULO SU NUEVO QUANTUM Y ELIJO SI LO PONGO EN LA LISTA READY PRIORIDAD O LA LISTA READY. HECHO
	// SI EL PROCESO ES DESLOJADO POR UNA INTERRUPCION, NO LO METO EN LA LISTA READY PRIORIDAD. HECHO  

	t_pcb* un_pcb = NULL;

	pthread_mutex_lock(&mutex_lista_exec);
	pthread_mutex_lock(&mutex_lista_ready);
	pthread_mutex_lock(&mutex_lista_ready_prioridad);
	
	// ELEGIR QUIEN EJECUTA

	// SI LA LISTA DE EXEC ESTA VACÍA, ME FIJO SI LA LISTA DE READY CON PRIORIDAD ESTA VACIA. HECHO
	// SI LA LISTA DE READY CON PRIORIDAD NO ESTA VACÍA, EJECUTA ESE. HECHO
	// SINO EJECUTA EL DE LA LISTA DE READY NORMAL- HECHO

	// SI EL PROCESO QUE VUELVE DE BLOCKED VENIA DE READY PRIORIDAD, EL NUEVO QUANTUM SERÁ EL ORIGINAL.HECHO

	// CUANDO GENERO UNA INTERRUPCIÓN DE DESALOJO POR HABER UN PROCESO CON MÁS PRIORIDAD, 
	// DEBO FRENAR EL TIMER DEL PROCESO DESALOJADO Y REANUDARLO CUANDO VUELVA A EXEC.

	if(list_is_empty(lista_execute) && !list_is_empty(lista_ready_prioridad)) {

		un_pcb = list_remove(lista_ready_prioridad,0);
		_enviar_pcb_CPU_dispatch(un_pcb);
		log_trace(kernel_log_debug, "COMIENZA A EJECUTAR EL PROCESO CON MAS PRIORIDAD (DE LA LISTA DE READY PRIORIDAD)");
		list_add(lista_execute, un_pcb);
		un_pcb->verificador = generar_verificador();
		cambiar_estado(un_pcb, EXEC);
		log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: READY_PRIORIDAD - Estado Actual: EXEC", un_pcb->pid);


		un_pcb->flag_proceso_venia_de_ready_prioridad = true;

		un_pcb->flag_cancelar_quantum = false;
		ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, un_pcb);

	} else if(list_is_empty(lista_execute) && !list_is_empty(lista_ready)){
		
		un_pcb = list_remove(lista_ready,0);
		_enviar_pcb_CPU_dispatch(un_pcb);
		log_trace(kernel_log_debug, "NO HAY NADIE EN READY PRIORIDAD, EJECUTA EL SIGUIENTE DE READY");
		list_add(lista_execute, un_pcb);
		un_pcb->verificador = generar_verificador();
		cambiar_estado(un_pcb, EXEC);
		log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: READY - Estado Actual: EXEC", un_pcb->pid);


		un_pcb->flag_proceso_venia_de_ready_prioridad = false;

		if(!(un_pcb->flag_proceso_desalojado_por_prioridad))
		{
			if (un_pcb != NULL && un_pcb->timer != NULL) {
        		temporal_destroy(un_pcb->timer);
   			}
			un_pcb->timer = temporal_create();
			log_trace(kernel_log_debug, "ARRANCA EL TIMER DEL PCB %d", un_pcb->pid);
		}
		else{
			temporal_resume(un_pcb->timer);
			un_pcb->flag_proceso_desalojado_por_prioridad = false;
		}
						
		un_pcb->flag_cancelar_quantum = false;

		ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, un_pcb);
	}else if(list_is_empty(lista_execute) && list_is_empty(lista_ready) && list_is_empty(lista_ready_prioridad)){
		log_warning(kernel_log_debug, "Lista de READY vacía");
	}

	// ELEGIR SI DESALOJO O NO

	// SI HAY ALGUN ELEMENTO EJECUTANDO EN EXEC Y LA LISTA READY CON PRIORIDAD NO ESTA VACIA, LANZO UNA INTERRUPCIÓN. HECHO
	// ADEMÁS SE DEBE CUMPLIR QUE EL PROCESO QUE ESTÁ EJECUTANDO NO VENIA DE READY PRIORIDAD. HECHO
	
	if(!list_is_empty(lista_execute)){
		t_pcb* pcb_execute = list_get(lista_execute, 0);
		if(!list_is_empty(lista_ready_prioridad) && (pcb_execute->quantum >= QUANTUM)) {
			
			log_trace(kernel_log_debug, "ENVIO INTERRUPCION PARA DESALOJO POR PRIORIDAD AL PID %d", pcb_execute->pid);
			sem_post(&sem_enviar_interrupcion_prioridad);
		}
	}
	
	pthread_mutex_unlock(&mutex_lista_ready_prioridad);
	pthread_mutex_unlock(&mutex_lista_ready);
	pthread_mutex_unlock(&mutex_lista_exec);

}	

void pcp_planificar_corto_plazo(){
	pausador();
	int flag_lista_ready_vacia = 0;

	pthread_mutex_lock(&mutex_lista_ready);
	if(list_is_empty(lista_ready)){
		flag_lista_ready_vacia = 1;
	}
	pthread_mutex_unlock(&mutex_lista_ready);

	if(flag_lista_ready_vacia == 0){


		switch (ALGORITMO_PLANIFICACION) {
			case FIFO:
				_atender_RR_FIFO();
				break;
			case ROUNDROBIN:
				_atender_RR_FIFO();
				break;
			case VIRTUALROUNDROBIN:
				_atender_VRR();
				break;
			default:
				log_error(kernel_log_debug, "ALGORITMO DE CORTO PLAZO DESCONOCIDO");
				exit(EXIT_FAILURE);
				break;
		}
	}
}
