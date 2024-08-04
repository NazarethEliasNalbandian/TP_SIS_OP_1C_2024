#include "../include/planificador_largo_plazo.h"
#include "../include/planificador_corto_plazo.h"
#include "../include/pcb.h"
#include "../include/servicios_kernel.h"

void plp_planifica(){
	t_pcb* un_pcb = NULL;

	pthread_mutex_lock(&mutex_lista_new);

	sem_wait(&sem_procesos_en_memoria);

	//Remover PCB de NEW si existen elementos
	un_pcb = list_remove(lista_new, 0); //Sale por FIFO

	if(un_pcb != NULL){

		// //Enviar Mensaje a memoria para que inicialice estructuras
		t_paquete* un_paquete = __crear_super_paquete(ESTRUCTURA_INICIADA_KERNEL_MEMORIA);
		__cargar_string_al_super_paquete(un_paquete, un_pcb->path);
		__cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
		enviar_paquete(un_paquete, fd_memoria);

		sem_wait(&sem_estructura_iniciada);

		//Agregando PCB a READY

		pthread_mutex_lock(&mutex_lista_ready);

		cambiar_estado(un_pcb, READY);
		list_add(lista_ready, un_pcb);

		log_info(kernel_log_obligatorio, " PID: %d - Estado Anterior: %s - Estado Actual: %s", un_pcb->pid, estado_to_string(NEW), estado_to_string(un_pcb->estado));
		char* pids_en_ready = lista_pids_en_ready(); // Chequear el ampersand
		log_info(kernel_log_obligatorio, "Cola Ready / Ready Prioridad: %s", pids_en_ready);

		if(pids_en_ready != NULL){

			free(pids_en_ready);
			pids_en_ready = NULL;
		}

		pthread_mutex_unlock(&mutex_lista_ready);

		eliminar_paquete(un_paquete);
	}

	pthread_mutex_unlock(&mutex_lista_new);

	ejecutar_en_un_hilo_nuevo_detach((void*)pcp_planificar_corto_plazo, NULL);
}


void plp_planificar_proceso_nuevo(t_pcb* un_pcb){

	if(un_pcb != NULL){
		//Asignando PCB al estado NEW
		pthread_mutex_lock(&mutex_lista_new);
		list_add(lista_new, un_pcb);
		un_pcb->estado = NEW;
		pthread_mutex_unlock(&mutex_lista_new);
		log_info(kernel_log_obligatorio, "Se crea el proceso %d en NEW", un_pcb->pid);

	}
	pausador();
	//Verificar si puedo pasar a READY segun el GMMP
	ejecutar_en_un_hilo_nuevo_detach((void*)plp_planifica, NULL);
}

void plp_exit(t_pcb* pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	//Este control verifica que ese PCB siga en la lista
	if(list_remove_element(lista_execute, pcb)){
		liberar_recursos_pcb(pcb);
		avisar_a_memoria_para_liberar_estructuras(pcb);
		sem_wait(&sem_estructura_liberada);
		transferir_from_actual_to_siguiente(pcb, lista_exit, mutex_lista_exit, EXIT);

		log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));

		log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", pcb->pid, motivo_to_string(pcb->motivo_exit));

		sem_post(&sem_procesos_en_memoria);
	}else{
		log_error(kernel_log_debug, "PCB no encontrada en EXEC [Eliminacion por consola]");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutex_lista_exec);

	pcp_planificar_corto_plazo();
}

/*Planificador a Largo Plazo para [XX] -> EXIT*/

void plp_planificar_proceso_exit(int pid){
	// Solo remuevo los que estan en: NEW-READY-BLOCKED, si esta en: EXEC-EXIT
	// se procede de otra manera.
	t_pcb* un_pcb = buscar_y_remover_pcb_por_pid(pid);


	if(un_pcb->estado == EXEC && un_pcb->flag_proceso_desalojado){
		un_pcb->flag_finalizar_proceso = true;
		log_info(kernel_log_debug, "PRENDI FLAG FINALIZAR PROCESO");
	}

	pausador();
	if(un_pcb != NULL){
		switch(un_pcb->estado){
		case NEW:

			// MANDO EL PCB A MEMORIA PARA LIBERAR SUS ESTRUCTURAS
			// ESPERO LA CONFIRMACIÓN
			// MANDO EL PROCESO A LA LISTA EXIT
			pthread_mutex_lock(&mutex_lista_new);
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);
			log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));

			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_new);
			break;
		case READY:

			// MANDO EL PCB A MEMORIA PARA LIBERAR SUS ESTRUCTURAS
			// ESPERO LA CONFIRMACIÓN
			// MANDO EL PROCESO A LA LISTA EXIT

			pthread_mutex_lock(&mutex_lista_ready);

			liberar_recursos_pcb(un_pcb);
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

			log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));

			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_ready);

			//-----------------------
			sem_post(&sem_procesos_en_memoria);
			//-----------------------

			// pcp_planificar_corto_plazo();

			break;
		case READY_PRIORIDAD:

			// MANDO EL PCB A MEMORIA PARA LIBERAR SUS ESTRUCTURAS
			// ESPERO LA CONFIRMACIÓN
			// MANDO EL PROCESO A LA LISTA EXIT

			pthread_mutex_lock(&mutex_lista_ready_prioridad);

			liberar_recursos_pcb(un_pcb);
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

			log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));

			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_ready_prioridad);

			//-----------------------
			sem_post(&sem_procesos_en_memoria);
			//-----------------------

			// pcp_planificar_corto_plazo();

			break;
		case EXEC:

			// EL ENUNCIADO DICE QUE SI SE MANDA UN PROCESO DE EXEC A EXIT,
			// HAY QUE ENVIAR UNA INTERRUPCIÓN Y ESPERAR A QUE CPU RETORNE EL CONTEXTO 
			// ANTES DE LIBERAR EL PCB

			// log_info(kernel_log_debug, "FLAG PROCESO DESALOJADO %d", flag_proceso_desalojado);
			// log_info(kernel_log_debug, "FLAG FINALIZAR PROCESO %d", flag_finalizar_proceso);


//			Enviar un interrupt
			un_pcb->flag_finalizar_proceso = true;
			log_info(kernel_log_debug, "Envio interrupcion");
			sem_post(&sem_enviar_interrupcion_consola);

			break;
		case BLOCKED:

			// MANDO EL PCB A MEMORIA PARA LIBERAR SUS ESTRUCTURAS
			// ESPERO LA CONFIRMACIÓN
			// MANDO EL PROCESO A LA LISTA EXIT
			
			pthread_mutex_lock(&mutex_lista_blocked);
			liberar_recursos_pcb(un_pcb);
			avisar_a_memoria_para_liberar_estructuras(un_pcb);
			sem_wait(&sem_estructura_liberada);
			transferir_from_actual_to_siguiente(un_pcb, lista_exit, mutex_lista_exit, EXIT);

			log_trace(kernel_log_debug, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));

			log_info(kernel_log_obligatorio, "Finaliza el proceso [PID: %d] - Motivo: %s", un_pcb->pid, motivo_to_string(un_pcb->motivo_exit));
			pthread_mutex_unlock(&mutex_lista_blocked);

			sem_post(&sem_procesos_en_memoria);

			// pcp_planificar_corto_plazo();

			break;
		case EXIT:

			pthread_mutex_lock(&mutex_lista_exit);
			//Este control verifica que esa PCB siga en la lista

			if(esta_pcb_en_una_lista_especifica(lista_exit, un_pcb)){
				log_warning(kernel_log_debug, "El PCB ya se encuentra en el estado EXIT");
			}else{
				log_error(kernel_log_debug, "El PCB_%d No se encuentra en la lista EXIT", un_pcb->pid);
				exit(EXIT_FAILURE);
			}
			pthread_mutex_unlock(&mutex_lista_exit);
			break;
		default:
			log_error(kernel_log_debug, "El PCB no tiene ESTADO");
			break;
		}
	}else
		log_error(kernel_log_debug, "CONSOLA - No se encontro el PID en ningun lado");
}

