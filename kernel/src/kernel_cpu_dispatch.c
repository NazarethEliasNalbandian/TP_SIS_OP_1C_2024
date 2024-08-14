#include "../include/kernel_cpu_dispatch.h"
#include "../include/kernel_cpu_interrupt.h"
#include "../include/pcb.h"
#include "../include/recurso.h"
#include "../include/entrada_salida.h"
#include "../include/servicios_kernel.h"
#include "../include/planificador_largo_plazo.h"
#include "../include/planificador_corto_plazo.h"

void atender_signal(t_pcb* pcb,char* recurso_a_liberar){
    log_info(kernel_log_debug, "Voy a buscar recurso: %s", recurso_a_liberar);
	t_recurso* recurso_buscado = buscar_recurso(recurso_a_liberar);

	if(recurso_buscado != NULL  && list_remove_element(pcb->lista_recursos_pcb, recurso_buscado)){
		recurso_buscado->pcb_asignado = NULL;
		recurso_buscado->instancias ++;
		log_info(kernel_log_debug, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, recurso_buscado->recurso_name, recurso_buscado->instancias);

		t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
        cargar_string_al_paquete(un_paquete,"1");
        enviar_paquete(un_paquete, fd_cpu_dispatch);
        eliminar_paquete(un_paquete);

		if(!list_is_empty(recurso_buscado->lista_bloqueados)){
			asignar_recurso_liberado_pcb(recurso_buscado);
		}
    }
	else{
		t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
    	cargar_string_al_paquete(un_paquete,"-1");
        enviar_paquete(un_paquete, fd_cpu_dispatch);
        eliminar_paquete(un_paquete);
		pcb->motivo_exit = INVALID_RESOURCE;
		plp_exit(pcb); //revisar
	}
}

void atender_wait(t_pcb* pcb,char* recurso_solicitado){
	t_recurso* recurso_buscado = buscar_recurso(recurso_solicitado);
	
	// Verifico que exista el recurso, en caso de no hacerlo el proceso se envia a EXIT
	if(recurso_buscado != NULL){
		if(recurso_buscado->instancias > 0){
			recurso_buscado->instancias --;
			list_add(pcb->lista_recursos_pcb, recurso_buscado);
			recurso_buscado->pcb_asignado = pcb;

			log_info(kernel_log_debug, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, recurso_buscado->recurso_name, recurso_buscado->instancias);

			t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
            cargar_string_al_paquete(un_paquete, "1");
            enviar_paquete(un_paquete, fd_cpu_dispatch);
            eliminar_paquete(un_paquete);

		}else{
			if(ALGORITMO_PLANIFICACION == VIRTUALROUNDROBIN && (pcb->timer->status != TEMPORAL_STATUS_STOPPED))
				temporal_stop(pcb->timer);

			log_info(kernel_log_debug, "RECURSO ENCONTRADO PERO NO HAY INSTANCIAS DISPONIBLES");
			

			// NO HAY SUFICIENTES INSTANCIAS DE RECURSO
			// ENVIAMOS -1 A CPU, PARA QUE DESALOJE EL PROCESO 
            t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
            cargar_string_al_paquete(un_paquete, "-1");
            enviar_paquete(un_paquete, fd_cpu_dispatch);
            eliminar_paquete(un_paquete);

			pthread_mutex_lock(&mutex_lista_exec);
			list_remove(lista_execute,0);
			pthread_mutex_unlock(&mutex_lista_exec);

			list_add(pcb->lista_recursos_pcb, recurso_buscado);

			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			agregar_pcb_a_lista_recurso(pcb ,recurso_buscado->lista_bloqueados, recurso_buscado->mutex_bloqueados);
            log_info(kernel_log_obligatorio, "PID: %d - Bloqueado por: %s", pcb->pid, recurso_solicitado);
			// sem_post(&sem_nuevo_en_block); da la señal para deteccion de deadlock, revisar

			pcp_planificar_corto_plazo();  
		}
	}else{
		log_info(kernel_log_debug, "EL RECURSO NO FUE ENCONTRADO");
        transferir_from_actual_to_siguiente(pcb, lista_exit, mutex_lista_exit, EXIT);
			
        t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
        cargar_string_al_paquete(un_paquete, "-1");
        enviar_paquete(un_paquete, fd_cpu_dispatch);
        eliminar_paquete(un_paquete);

		pcb->motivo_exit = INVALID_RESOURCE;
		plp_exit(pcb);  //revisar
	}

}

void atender_io_gen_sleep(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int unidades_trabajo) {
    
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->operacion_solicitada = IO_GEN_SLEEP;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A EXIT
    if (!existe_interfaz(instancia_entradasalida->nombre)) {
		log_warning(kernel_log_debug, "INTERFAZ NO EXISTE");
		pcb->motivo_exit = INVALID_INTERFACE;
		if(pcb_con_mochila != NULL){

			destruir_pcb_con_mochila(pcb_con_mochila);
			pcb_con_mochila = NULL;
		}
		if(mochila != NULL){

			destruir_mochila(mochila);
			mochila = NULL;
		}
        plp_exit(pcb);
    } else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) {
		log_warning(kernel_log_debug, "INTERFAZ NO PUEDE REALIZAR LA OPERACION");
        // ENVÍO PROCESO A EXIT
		pcb->motivo_exit = INVALID_INTERFACE;
		if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		if(mochila != NULL){
			destruir_mochila(mochila);
			mochila = NULL;
		}
        plp_exit(pcb);
    } else {
		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_gen_sleep(pcb->pid, unidades_trabajo, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();

			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion IO_STDIN_READ");
				if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(pcb_con_mochila != NULL){
			    	destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		    }
			}
		}
		else
		{
			pcb_con_mochila->mochila = mochila;
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
    }
}

void atender_io_stdin(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_STDIN_READ;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A EXIT
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			// destruir_pcb_con_mochila(pcb_con_mochila);
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			// destruir_pcb_con_mochila(pcb_con_mochila);
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO IO STDIN READ");

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_stdin(pcb->pid, tamanio, direccion_fisica, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion IO_STDIN_READ");
				if(pcb_con_mochila != NULL){
			   		destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		    	}
			}
		}
		else
		{
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}

void atender_io_stdout(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_STDOUT_WRITE;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO IO STDOUT WRITE");

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_stdout(pcb->pid, tamanio, direccion_fisica, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion IO_STDIN_READ");
				if(pcb_con_mochila != NULL){
			   		destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		    	}
			}
		}
		else
		{
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}

void atender_io_fs_create(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo){
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_FS_CREATE;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_fs_create(pcb->pid, nombre_archivo, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));
				if(pcb_con_mochila != NULL){
			    	destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		   	 }
			}
		}
		else
		{
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}

void atender_io_fs_delete(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_FS_DELETE;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_fs_delete(pcb->pid, nombre_archivo, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));
				if(pcb_con_mochila != NULL){
			    	destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		    }
			}
		}
		else
		{

			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}

void atender_io_fs_truncate(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo, size_t tamanio){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_FS_TRUNCATE;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_fs_truncate(pcb->pid, nombre_archivo, tamanio, instancia_entradasalida->socket);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));
				if(pcb_con_mochila != NULL){
			   	 	destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		   		 }
			}
		}
		else
		{
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}

void atender_io_fs_write(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio, char* nombre_archivo, int puntero_archivo){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_FS_WRITE;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_fs_write(pcb->pid, tamanio, direccion_fisica, instancia_entradasalida->socket, nombre_archivo, puntero_archivo);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));
				if(pcb_con_mochila != NULL){
			    	destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		    	}
			}
		}
		else
		{
			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}



void atender_io_fs_read(t_pcb* pcb,t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio, int puntero_archivo, char* nombre_archivo){
  
	t_pcb_con_mochila* pcb_con_mochila = malloc(sizeof(t_pcb_con_mochila));
	pcb_con_mochila->pcb = duplicar_pcb(pcb);
	pcb_con_mochila->mochila = mochila;
	pcb_con_mochila->operacion_solicitada = IO_FS_READ;

	// VALIDAR QUE EXISTE LA INTERFAZ, SINO ENVIAR PROCESO A READY
	if(!existe_interfaz(instancia_entradasalida->nombre))
		{
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else if (!admite_operacion_solicitada(instancia_entradasalida->tipo, pcb_con_mochila->operacion_solicitada)) 
		{
			// ENVÍO PROCESO A EXIT
			pcb->motivo_exit = INVALID_INTERFACE;
			plp_planificar_proceso_exit(pcb->pid);
			if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
		}
	else {
		// ENVIAR PROCESO A BLOCKED 
		// Y ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA

		if(instancia_entradasalida->libre){;
			// SACAR DE LA LISTA DE EXEC
			if (list_remove_element(lista_execute, pcb)) {
				log_warning(kernel_log_debug, "ATIENDO %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));

				// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
				transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
				log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

				// ASIGNO EL PROCESO A LA INSTANCIA DE ENTRADA SALIDA
				instancia_entradasalida->pcb_con_interfaz_asignada = pcb_con_mochila;
				instancia_entradasalida->libre = 0;

				// ENVIAR MENSAJE A LA ENTRADA SALIDA PARA QUE EJECUTE LA OPERACION DE ENTRADA SALIDA
				enviar_pcb_entradasalida_io_fs_read(pcb->pid, tamanio, puntero_archivo, direccion_fisica, instancia_entradasalida->socket, nombre_archivo);

				// REPLANIFICO
				pcp_planificar_corto_plazo();
			} else {
				log_error(kernel_log_debug, "No se pudo remover el proceso de EXECUTE al ejecutar la instruccion %s", operacion_to_string(pcb_con_mochila->operacion_solicitada));
				if(pcb_con_mochila != NULL){
			   		destruir_pcb_con_mochila(pcb_con_mochila);
			    	pcb_con_mochila = NULL;
		   	 }
			}
		}
		else
		{

			list_remove_element(lista_execute, pcb);
			// PONER ESTADO DEL PCB EN BLOCKED y PONER EN LA LISTA DE BLOCKED GENERAL
			transferir_from_actual_to_siguiente(pcb, lista_blocked, mutex_lista_blocked, BLOCKED);
			log_blocked_proceso(pcb->pid, instancia_entradasalida->nombre);

			// PONER EN LA COLA DE ESPERA ASOCIADA A ESTA INTERFAZ
			pthread_mutex_lock(&(instancia_entradasalida->mutex_espera));
			queue_push(instancia_entradasalida->cola_de_espera, pcb_con_mochila);
			pthread_mutex_unlock(&(instancia_entradasalida->mutex_espera));
		}
	}
}


// ENVÍOS

void enviar_pcb_entradasalida_io_gen_sleep(int pid, int unidades_trabajo, int fd_instancia_entradasalida){

	t_paquete* un_paquete = crear_paquete(PCB_SLEEP);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_int_al_paquete(un_paquete, unidades_trabajo);
	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_stdin(int pid, size_t tamanio, int direccion_fisica, int fd_instancia_entradasalida){
	
	t_paquete* un_paquete = crear_paquete(PCB_STDIN);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_size_t_al_paquete(un_paquete, tamanio);
	cargar_int_al_paquete(un_paquete, direccion_fisica);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_stdout(int pid, size_t tamanio, int direccion_fisica, int fd_instancia_entradasalida){
	
	t_paquete* un_paquete = crear_paquete(PCB_STDOUT);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_size_t_al_paquete(un_paquete, tamanio);
	cargar_int_al_paquete(un_paquete, direccion_fisica);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_fs_create(int pid, char* nombre_archivo, int fd_instancia_entradasalida){
	t_paquete* un_paquete = crear_paquete(PCB_FS_CREATE);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_string_al_paquete(un_paquete, nombre_archivo);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);

}
void enviar_pcb_entradasalida_io_fs_delete(int pid, char* nombre_archivo, int fd_instancia_entradasalida){
	t_paquete* un_paquete = crear_paquete(PCB_FS_DELETE);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_string_al_paquete(un_paquete, nombre_archivo);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_fs_truncate(int pid, char* nombre_archivo, size_t tamanio, int fd_instancia_entradasalida){
	t_paquete* un_paquete = crear_paquete(PCB_FS_TRUNCATE);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_string_al_paquete(un_paquete, nombre_archivo);
	cargar_int_al_paquete(un_paquete, tamanio);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_fs_write(int pid, size_t tamanio, int direccion_fisica, int fd_instancia_entradasalida, char* nombre_archivo, int puntero_archivo){
	
	t_paquete* un_paquete = crear_paquete(PCB_FS_WRITE);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_string_al_paquete(un_paquete, nombre_archivo);
	cargar_size_t_al_paquete(un_paquete, tamanio);
	cargar_int_al_paquete(un_paquete, puntero_archivo);
	cargar_int_al_paquete(un_paquete, direccion_fisica);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void enviar_pcb_entradasalida_io_fs_read(int pid, size_t tamanio, int puntero_archivo, int direccion_fisica, int fd_instancia_entradasalida, char* nombre_archivo){
	
	t_paquete* un_paquete = crear_paquete(PCB_FS_READ);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_string_al_paquete(un_paquete, nombre_archivo);
	cargar_size_t_al_paquete(un_paquete, tamanio);
	cargar_int_al_paquete(un_paquete, puntero_archivo);
	cargar_int_al_paquete(un_paquete, direccion_fisica);

	enviar_paquete(un_paquete, fd_instancia_entradasalida);
	eliminar_paquete(un_paquete);
}

void atender_kernel_cpu_dispatch(){
   bool control_key=1;
   while (control_key){
       int cod_op = recibir_operacion(fd_cpu_dispatch);
        log_info(kernel_log_debug, "Se recibio algo de CPU_Dispatch");
        t_pcb* pcb = NULL;
        t_buffer* unBuffer = NULL;
        t_contexto* contexto = malloc(sizeof(t_contexto));
        contexto->r_cpu = malloc(sizeof(t_registrosCPU));
        t_mochila* mochila = NULL;
        t_mochila* copia_mochila = NULL;

       switch (cod_op) {
       	case MENSAJE:
           recibir_mensaje(fd_cpu_dispatch,kernel_log_debug);
		   destruir_buffer(unBuffer);
		   destruir_contexto_por_param(contexto);
           break;
        case ATENDER_INSTRUCCION_CPU:
			unBuffer = recibir_paquete(fd_cpu_dispatch);
			recibir_contexto(unBuffer, contexto);
		   if(mochila != NULL)
		   {
			 	destruir_mochila(mochila); 
		   }
			mochila = crear_mochila();
			recibir_mochila(unBuffer, mochila, kernel_log_debug);
			destruir_buffer(unBuffer);
			pausador();

			pcb = obtener_proceso_desalojado(contexto);
			destruir_contexto_por_param(contexto);

            if(pcb==NULL){
                t_paquete* un_paquete = crear_paquete(ATENDER_RTA_KERNEL);
				cargar_string_al_paquete(un_paquete, "-1");
                enviar_paquete(un_paquete, fd_cpu_dispatch);
                eliminar_paquete(un_paquete);
                if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}
                break; //espero q este break termine el case
            }
            pcb->flag_cancelar_quantum = true;

			if((ALGORITMO_PLANIFICACION == VIRTUALROUNDROBIN) && proceso_ejecutara_io(mochila->instruccionAsociada))
			{

				if(pcb->timer != NULL)
					temporal_stop(pcb->timer);
				else
				{
					log_error(kernel_log_debug,"EL PCB %d NO TIENE TIMER CREADO", pcb->pid);
				}
			}

           pcb->flag_proceso_desalojado = true;


           //Esto sirve para darle prioridad al desalojo por consola
           if(pcb->flag_finalizar_proceso){
				// TIRA ERROR DE TWO FEW ARGUMENTS EN FUNCION DESALOJAR
                ejecutar_en_un_hilo_nuevo_detach((void*)_desalojar_proceso, pcb);
				if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "WAIT") == 0){
			   char* recurso_solicitado = (char*) queue_pop(mochila->parametros);
               log_info(kernel_log_debug, " If del WAIT %s", recurso_solicitado);
               atender_wait(pcb,recurso_solicitado);

			   if(recurso_solicitado != NULL){

             		free(recurso_solicitado);
					recurso_solicitado = NULL;
			   }
			    if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "SIGNAL") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   char* recurso_a_liberar = (char*) queue_pop(mochila->parametros);
               atender_signal(pcb, recurso_a_liberar);
			   if(recurso_a_liberar != NULL){

               		free(recurso_a_liberar);
					recurso_a_liberar = NULL;
			   }
			   if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_GEN_SLEEP") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
				char* nombre_interfaz = (char*) queue_pop(mochila->parametros);
			    copia_mochila = copiar_mochila(mochila); 

				int* puntero_unidad_trabajo = (int*) queue_pop(mochila->parametros);
				int unidades_trabajo =  *puntero_unidad_trabajo;

				if(puntero_unidad_trabajo != NULL){

					free(puntero_unidad_trabajo);
					puntero_unidad_trabajo = NULL;
				}

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				if(instancia_entradasalida != NULL){

					atender_io_gen_sleep(pcb, copia_mochila, instancia_entradasalida, unidades_trabajo);
					// atender_io_gen_sleep(pcb, mochila, instancia_entradasalida, unidades_trabajo);
				}
				else	
					log_error(kernel_log_debug,"INSTANCIA NO EXISTE");
				if(mochila != NULL){

					destruir_mochila(mochila);
					mochila = NULL;
				}

				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_STDIN_READ") == 0){
                log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
				char* nombre_interfaz = queue_pop(mochila->parametros);
				copia_mochila = copiar_mochila(mochila); 
				size_t* puntero_tamanio = (size_t*) queue_pop(mochila->parametros);
				size_t tamanio = *puntero_tamanio;

				// if(puntero_tamanio != NULL){

				// 	free(puntero_tamanio);
				// 	puntero_tamanio = NULL;
				// }

				// OBTENER TODAS LAS DIRECCIONES FISICAS
				int * puntero_direccion_fisica = (int*) queue_pop(mochila->parametros);
				int direccion_fisica = *puntero_direccion_fisica;

				if(puntero_direccion_fisica != NULL){

					//free(puntero_direccion_fisica);
					puntero_direccion_fisica = NULL;
				}

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_stdin(pcb, copia_mochila, instancia_entradasalida, direccion_fisica, tamanio);
				// atender_io_stdin(pcb, mochila, instancia_entradasalida, direccion_fisica, tamanio);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}

				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_STDOUT_WRITE") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
				char* nombre_interfaz = queue_pop(mochila->parametros);
			    copia_mochila = copiar_mochila(mochila); 

				size_t* puntero_tamanio = (size_t*) queue_pop(mochila->parametros);
				size_t tamanio = *puntero_tamanio;

				if(puntero_tamanio != NULL){

					free(puntero_tamanio);
					puntero_tamanio = NULL;
				}

				int * puntero_direccion_fisica = (int*) queue_pop(mochila->parametros);
				int direccion_fisica = *puntero_direccion_fisica;

				if(puntero_direccion_fisica != NULL){

					free(puntero_direccion_fisica);
					puntero_direccion_fisica = NULL;
				}

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_stdout(pcb, copia_mochila, instancia_entradasalida, direccion_fisica, tamanio);
				// atender_io_stdout(pcb, mochila, instancia_entradasalida, direccion_fisica, tamanio);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}

				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_FS_CREATE") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   char* nombre_interfaz = queue_pop(mochila->parametros);
			   copia_mochila = copiar_mochila(mochila);
			   char* nombre_archivo = queue_pop(mochila->parametros);

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_fs_create(pcb, copia_mochila, instancia_entradasalida, nombre_archivo);
				// atender_io_fs_create(pcb, mochila, instancia_entradasalida, nombre_archivo);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_FS_DELETE") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   	char* nombre_interfaz = queue_pop(mochila->parametros);
				copia_mochila = copiar_mochila(mochila);
			    char* nombre_archivo = queue_pop(mochila->parametros);

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_fs_delete(pcb, copia_mochila, instancia_entradasalida, nombre_archivo);
				// atender_io_fs_delete(pcb, mochila, instancia_entradasalida, nombre_archivo);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_FS_TRUNCATE") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   	char* nombre_interfaz = queue_pop(mochila->parametros);
				copia_mochila = copiar_mochila(mochila);
			    char* nombre_archivo = queue_pop(mochila->parametros);

				size_t* puntero_tamanio = (size_t*) queue_pop(mochila->parametros);
				size_t tamanio = *puntero_tamanio;

				if(puntero_tamanio != NULL){

					free(puntero_tamanio);
					puntero_tamanio = NULL;
				}


				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_fs_truncate(pcb, copia_mochila, instancia_entradasalida, nombre_archivo, tamanio);
				// atender_io_fs_truncate(pcb, mochila, instancia_entradasalida, nombre_archivo, tamanio);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp( mochila->instruccionAsociada, "IO_FS_WRITE") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   	char* nombre_interfaz = queue_pop(mochila->parametros);
				copia_mochila = copiar_mochila(mochila);
				char* nombre_archivo = queue_pop(mochila->parametros);

								
				int * puntero_puntero_archivo = (int*) queue_pop(mochila->parametros);
				int puntero_archivo = *puntero_puntero_archivo;

				if(puntero_puntero_archivo != NULL){

					free(puntero_puntero_archivo);
					puntero_puntero_archivo = NULL;
				}


				size_t* puntero_tamanio = (size_t*) queue_pop(mochila->parametros);
				size_t tamanio = *puntero_tamanio;
				if(puntero_tamanio != NULL){

					free(puntero_tamanio);
					puntero_tamanio = NULL;
				}
				
				// OBTENER TODAS LAS DIRECCIONES FISICAS
				int * puntero_direccion_fisica = (int*) queue_pop(mochila->parametros);
				int direccion_fisica = *puntero_direccion_fisica;
				if(puntero_direccion_fisica != NULL){

					free(puntero_direccion_fisica);
					puntero_direccion_fisica = NULL;
				}

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);

				atender_io_fs_write(pcb, copia_mochila, instancia_entradasalida, direccion_fisica, tamanio, nombre_archivo, puntero_archivo);
				// atender_io_fs_write(pcb, mochila, instancia_entradasalida, direccion_fisica, tamanio, nombre_archivo, puntero_archivo);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}

           }else if(strcmp(mochila->instruccionAsociada, "IO_FS_READ") == 0){
               log_info(kernel_log_debug, "Entre a: %s", mochila->instruccionAsociada);
			   	char* nombre_interfaz = queue_pop(mochila->parametros);
				copia_mochila = copiar_mochila(mochila);
				char* nombre_archivo = queue_pop(mochila->parametros);
				
				int * puntero_puntero_archivo = (int*) queue_pop(mochila->parametros);
				int puntero_archivo = *puntero_puntero_archivo;
				if(puntero_puntero_archivo != NULL){

					free(puntero_puntero_archivo);
					puntero_puntero_archivo = NULL;
				}

				size_t* puntero_tamanio = (size_t*) queue_pop(mochila->parametros);
				size_t tamanio = *puntero_tamanio;

				if(puntero_tamanio != NULL){

					free(puntero_tamanio);
					puntero_tamanio = NULL;
				}

				int * puntero_direccion_fisica = (int*) queue_pop(mochila->parametros);
				int direccion_fisica = *puntero_direccion_fisica;
				if(puntero_direccion_fisica != NULL){

					free(puntero_direccion_fisica);
					puntero_direccion_fisica = NULL;
				}

				t_instancia_io* instancia_entradasalida = obtener_instancia_por_nombre(nombre_interfaz);


				atender_io_fs_read(pcb,copia_mochila, instancia_entradasalida, direccion_fisica, tamanio, puntero_archivo, nombre_archivo);
				// atender_io_fs_read(pcb, mochila, instancia_entradasalida, direccion_fisica, tamanio, puntero_archivo, nombre_archivo);
				if(mochila != NULL){
					destruir_mochila(mochila);
					mochila = NULL;
				}
				if(nombre_interfaz != NULL){

					free(nombre_interfaz);
					nombre_interfaz = NULL;
				}
           }

			if(mochila != NULL)
		   	{
			 	destruir_mochila(mochila); 
				mochila = NULL;

		   	}
           break;
       case ATENDER_DESALOJO_PROCESO_CPU:
		   unBuffer = recibir_paquete(fd_cpu_dispatch);
	       recibir_contexto(unBuffer, contexto);

		   pcb = obtener_proceso_desalojado(contexto);

		   if(pcb==NULL)
		   {
			log_info(kernel_log_debug,"PCB ES NULL ATENDER PROCESO, SE CANCELA LA INTERRUPCION");
		   	break;
		   }
		   	pcb->flag_cancelar_quantum = true;

		   
		   // SI ESTAMOS EN VRR Y EL PROCESO NO VENIA DE READY PRIORIDAD, FRENO EL TIMER
		   // SI EL PROCESO VENIA DE READY PRIORIDAD, NO INICIE EL TIMER
		   
		   if((ALGORITMO_PLANIFICACION == VIRTUALROUNDROBIN) && !(pcb->flag_proceso_venia_de_ready_prioridad)){

				if (pcb->timer != NULL) {
            		temporal_stop(pcb->timer);
       			} else {
            		log_warning(kernel_log_debug, "El timer no ha sido inicializado.");
        		}
		   }
		   destruir_contexto_por_param(contexto);
		    char* motivo_desalojo = recibir_string_del_buffer(unBuffer);
			destruir_buffer(unBuffer);

           pcb->flag_proceso_desalojado = true;

           log_warning(kernel_log_debug, "<PID:%d>[%s]", pcb->pid, motivo_desalojo);

		   if(strcmp(motivo_desalojo, "PRIORIDAD") == 0){
				pcb->flag_proceso_desalojado_por_prioridad = true;
		   }

           _atender_motivo_desalojo(motivo_desalojo, pcb);

			if(motivo_desalojo != NULL){

		   		free(motivo_desalojo);
				motivo_desalojo = NULL;
			}
           break;
        case ATENDER_OUT_OF_MEMORY:
			log_info(kernel_log_debug, "DESALOJO OUT OF MEMORY");
			unBuffer = recibir_paquete(fd_cpu_dispatch);
			recibir_contexto(unBuffer, contexto);
			destruir_buffer(unBuffer);

			pcb = obtener_proceso_desalojado(contexto);
			destruir_contexto_por_param(contexto);

			if(pcb==NULL){
				log_error(kernel_log_debug,"PCB ES NULL ATENDER OUT OF MEMORY");
		   		break;
		    }

			pcb->motivo_exit = OUT_OF_MEMORY;

            pcb->flag_proceso_desalojado = true;

            pcb->flag_cancelar_quantum = true;

			plp_exit(pcb);

			// pcb->flag_proceso_desalojado = false;
           break;
        case ATENDER_EXIT:
			log_info(kernel_log_debug, "DESALOJO EXIT");
			unBuffer = recibir_paquete(fd_cpu_dispatch);
			recibir_contexto(unBuffer, contexto);
			destruir_buffer(unBuffer);

			pcb = obtener_proceso_desalojado(contexto);

			if(pcb==NULL){
				log_error(kernel_log_debug,"PCB ES NULL ATENDER EXIT");
		   		break;
		    }
            pcb->flag_cancelar_quantum = true;
			destruir_contexto_por_param(contexto);

            pcb->flag_proceso_desalojado = true;


			pcb->motivo_exit = SUCCESS;
			plp_exit(pcb);

			//pthread_mutex_lock(&mutex_flag_proceso_desalojado);
			// pcb->flag_proceso_desalojado = false;
			///pthread_mutex_unlock(&mutex_flag_proceso_desalojado);
           break;
       case -1:
           log_error(kernel_log_debug, "[DESCONEXION]");
           control_key = 0;
		   destruir_contexto_por_param(contexto);
           break;
       default:
           log_warning(kernel_log_debug, "Operacion desconocida");
		   destruir_contexto_por_param(contexto);
           break;
       }
   }
   log_warning(kernel_log_debug, "Saliendo del hilo de KERNEL_CPU_DISPATCH");
}


