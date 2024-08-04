#include "../include/pcb.h"
#include "../include/servicios_kernel.h"
#include "../include/planificador_corto_plazo.h"

// FUNCIONA
char* estado_to_string(int un_valor){
	switch (un_valor) {
		case NEW:
			return "NEW";
			break;
		case READY:
			return "READY";
			break;
		case EXEC:
			return "EXEC";
			break;
		case BLOCKED:
			return "BLOCKED";
			break;
		case EXIT:
			return "EXIT";
			break;
		case READY_PRIORIDAD:
			return "READY_PRIORIDAD";
			break;
		default:
			log_error(kernel_log_debug, "No se reconocio el nombre del estado");
			return "UNKNOWN";
			break;
	}
}


// FUNCIONA
t_pcb* crear_pcb(char* path){
	t_pcb* nuevo_PCB = malloc(sizeof(t_pcb));

	pthread_mutex_lock(&mutex_process_id);
	process_id++;
	nuevo_PCB->pid = process_id;
	pthread_mutex_unlock(&mutex_process_id);
	nuevo_PCB->verificador = 0;
    nuevo_PCB->quantum = QUANTUM;

	nuevo_PCB->path = string_duplicate(path);

	nuevo_PCB->lista_recursos_pcb = list_create();
	// pthread_mutex_init(&nuevo_PCB->mutex_lista_recursos, NULL);

	nuevo_PCB->registros_CPU = malloc(sizeof(t_registrosCPU));
	nuevo_PCB->registros_CPU->PC = 0;
	nuevo_PCB->registros_CPU->AX = 0;
	nuevo_PCB->registros_CPU->BX = 0;
	nuevo_PCB->registros_CPU->CX = 0;
	nuevo_PCB->registros_CPU->DX = 0;
    nuevo_PCB->registros_CPU->EAX = 0;
	nuevo_PCB->registros_CPU->EBX = 0;
	nuevo_PCB->registros_CPU->ECX = 0;
	nuevo_PCB->registros_CPU->EDX = 0;
    nuevo_PCB->registros_CPU->SI = 0;
	nuevo_PCB->registros_CPU->DI = 0;

	nuevo_PCB->flag_proceso_desalojado_por_prioridad = false;
   	nuevo_PCB->flag_proceso_venia_de_ready_prioridad = false;
	nuevo_PCB->flag_proceso_desalojado = false;
   	nuevo_PCB->flag_finalizar_proceso = false;
	nuevo_PCB->flag_cancelar_quantum = false;

	nuevo_PCB->timer = NULL;
	agrego_datos_pcb(nuevo_PCB);
	return nuevo_PCB;
}

// FUNCIONA
char* motivo_to_string(t_motivo_exit motivo_exit){

	switch(motivo_exit){
	case SUCCESS:
		return "SUCCESS";
		break;
	case INVALID_RESOURCE:
		return "INVALID_RESOURCE";
		break;
	case INVALID_INTERFACE:
		return "INVALID_INTERFACE";
		break;
	case OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
		break;
	case INTERRUPTED_BY_USER:
		return "INTERRUPTED_BY_USER";
		break;
	default:
		return "ERROR";
	}
}

// FUNCIONA
void cambiar_estado(t_pcb* un_pcb, est_pcb nex_state){
	un_pcb->estado = nex_state;
}

// FUNCIONA
void imprimir_pcb(t_pcb* un_pcb){
	log_info(kernel_log_debug, "<PCB_%d> [%s]",
							un_pcb->pid,
							un_pcb->path);
}


void _enviar_pcb_CPU_dispatch(t_pcb* un_pcb){

    t_paquete* un_paquete = __crear_super_paquete(EJECUTAR_PROCESO_KC);
	__cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
	__cargar_int_al_super_paquete(un_paquete, un_pcb->verificador);
	agregar_registros_a_paquete(un_paquete, un_pcb->registros_CPU);
    enviar_paquete(un_paquete, fd_cpu_dispatch);
    eliminar_paquete(un_paquete);
}

// FUNCIONA 
char* lista_pids_en_ready(){
    int id_process;
    char* pids_en_string = string_new();
    string_append(&pids_en_string, "[");

    for(int i = 0; i < list_size(lista_ready); i++){
        t_pcb* pcb = list_get(lista_ready, i);
        id_process = pcb->pid;
        char* id_str = string_itoa(id_process);
        if(i == 0){
            string_append(&pids_en_string, id_str);
        }else{
            string_append(&pids_en_string, ", ");
            string_append(&pids_en_string, id_str);
        }

		if(id_str != NULL){

        	free(id_str);  // Liberar la memoria de id_str
			id_str = NULL;
		}
    }

    string_append(&pids_en_string, " / ");

    for(int i = 0; i < list_size(lista_ready_prioridad); i++){
        t_pcb* pcb = list_get(lista_ready_prioridad, i);
        id_process = pcb->pid;
        char* id_str = string_itoa(id_process);
        if(i == 0){
            string_append(&pids_en_string, id_str);
        }else{
            string_append(&pids_en_string, ", ");
            string_append(&pids_en_string, id_str);
        }
        if(id_str != NULL){

        	free(id_str);  // Liberar la memoria de id_str
			id_str = NULL;
		}
    }

    string_append(&pids_en_string, "]");

    return pids_en_string;
}

void destruir_pcb(t_pcb* un_pcb){
    if (un_pcb == NULL) {
        return; // No hay nada que liberar si el puntero es NULL
    }

    // Liberar el path si no es NULL
    if (un_pcb->path != NULL) {
        free(un_pcb->path);
        un_pcb->path = NULL; // Evitar doble liberación
    }

    // Liberar registros_CPU si no es NULL
    if (un_pcb->registros_CPU != NULL) {
        free(un_pcb->registros_CPU);
        un_pcb->registros_CPU = NULL; // Evitar doble liberación
    }

    // Destruir la lista de recursos si no es NULL
    if (un_pcb->lista_recursos_pcb != NULL) {
        list_destroy(un_pcb->lista_recursos_pcb);
        un_pcb->lista_recursos_pcb = NULL; // Evitar doble liberación
    }

    // Destruir el mutex si es necesario y no está destruido
    // pthread_mutex_destroy(&un_pcb->mutex_lista_recursos);

    // Finalmente, liberar la estructura t_pcb
	if(un_pcb != NULL){

    	free(un_pcb);
		un_pcb = NULL;
	}
}

void destruir_mochila(t_mochila* mochila) {
    if (mochila == NULL) 
		return;

    // if (mochila->parametros != NULL) {
	// 	if(!queue_is_empty(mochila->parametros)){
    //     	// queue_clean_and_destroy_elements(mochila->parametros, (void*) safe_free);
	// 	}
    //     queue_destroy(mochila->parametros);
    //     mochila->parametros = NULL;
    // }

    if (mochila->instruccionAsociada != NULL) {
        free(mochila->instruccionAsociada);
        mochila->instruccionAsociada = NULL;
    }

	if(mochila != NULL){

	    free(mochila);
		mochila = NULL;
	}
}

void agrego_datos_pcb(t_pcb* un_pcb)
{
	if(ALGORITMO_PLANIFICACION == VIRTUALROUNDROBIN){

		un_pcb->quantum  += 600;
	}
}

void destruir_pcb_con_mochila(t_pcb_con_mochila* pcb_con_mochila) {
    if (pcb_con_mochila == NULL) {
        return;
    }

    // Destruir el PCB si no es NULL
    if (pcb_con_mochila->pcb != NULL) {
        destruir_pcb(pcb_con_mochila->pcb);
		pcb_con_mochila->pcb = NULL; 
    }

    // Destruir la mochila si no es NULL
    if (pcb_con_mochila->mochila != NULL) {
        destruir_mochila(pcb_con_mochila->mochila);
		pcb_con_mochila->mochila = NULL;
    }

    // Finalmente, liberar la estructura t_pcb_con_mochila
	if(pcb_con_mochila != NULL){

    	free(pcb_con_mochila);
		pcb_con_mochila = NULL;
	}
}

// FUNCIONA PERO NO REMUEVE EN EXEC Y EXIT, NO REQUIERE MUTEX ALREDEDOR
// BUSCA EL PCB EN CADA UNA DE LAS LISTAS DE CADA ESTADO Y LO REMUEVE
t_pcb* buscar_y_remover_pcb_por_pid(int un_pid){
	t_pcb* un_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	};

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_new);
		if(list_any_satisfy(lista_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_new, (void*)__buscar_pcb);
			list_remove_element(lista_new, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_new);
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_ready);
		if(list_any_satisfy(lista_ready, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_ready, (void*)__buscar_pcb);
			list_remove_element(lista_ready, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_ready);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exec);
		if(list_any_satisfy(lista_execute, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_execute, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exec);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_exit);
		if(list_any_satisfy(lista_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_exit);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_blocked);
		if(list_any_satisfy(lista_blocked, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_blocked, (void*)__buscar_pcb);
			list_remove_element(lista_blocked, un_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_blocked);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		un_pcb = NULL;
		log_error(kernel_log_debug, "PID no encontrada en ninguna lista");
	}
	return un_pcb;
}


void avisar_a_memoria_para_liberar_estructuras(t_pcb* un_pcb){

	t_paquete* un_paquete = __crear_super_paquete(ESTRUCTURA_LIBERADA_KERNEL_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, un_pcb->pid);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
	log_info(kernel_log_debug, "Mensaje a MEMORIA: ESTRUCTURA_LIBERADA_KERNEL_MEMORIA [PID: %d]", un_pcb->pid);
}

// FUNCIONA PERO NO REMUEVE. REMOVER ANTES DE USAR
void transferir_from_actual_to_siguiente(t_pcb* pcb, t_list* lista_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente){
    char* estado_anterior = estado_to_string(pcb->estado);
    char* siguente_estado = estado_to_string(estado_siguiente);

    cambiar_estado(pcb, estado_siguiente);

    agregar_pcb_lista(pcb, lista_siguiente, mutex_siguiente);

    log_info(kernel_log_obligatorio, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_anterior, siguente_estado);

    if((strcmp(siguente_estado, "READY") == 0) || (strcmp(siguente_estado, "READY_PRIORIDAD") == 0)){
        char* pids_en_ready = lista_pids_en_ready();
        log_info(kernel_log_obligatorio, "Cola Ready / Ready Prioridad: %s", pids_en_ready);

		if(pids_en_ready != NULL){

        	free(pids_en_ready);
			pids_en_ready = NULL;
		}
    }
}

void desbloquear_proceso_por_pid(int* pid_process){
//	Busco el pcb en la lista de bloqueados y lo elimino
	pausador();
	pthread_mutex_lock(&mutex_lista_blocked);
	t_pcb* pcb = buscar_pcb_por_pid_en(*pid_process, lista_blocked);
	if(pcb != NULL){
		list_remove_element(lista_blocked, pcb);
		analizarSiLoMandoAReadyOReadyPrioridad(pcb);
	}
	pthread_mutex_unlock(&mutex_lista_blocked);

	if(pid_process != NULL){

		free(pid_process);
		pid_process = NULL;
	}
	pcp_planificar_corto_plazo();
}

void asignar_recurso_liberado_pcb(t_recurso* un_recurso){
	pausador();

	pthread_mutex_lock(&un_recurso->mutex_bloqueados);

	if(!list_is_empty(un_recurso->lista_bloqueados)){
		t_pcb* pcb_liberado = list_remove(un_recurso->lista_bloqueados,0);

		un_recurso->pcb_asignado = pcb_liberado;
		un_recurso->instancias--;
		int* pid_process = malloc(sizeof(int));
		*pid_process = pcb_liberado->pid;
		desbloquear_proceso_por_pid(pid_process);
	}else
		log_warning(kernel_log_debug, "La lista de BLOQUEADOS del RECURSO esta vacia");

	pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
}

// void liberar_recursos_pcb(t_pcb* pcb){

// 	// TENGO QUE PARA CADA RECURSO, VER SI EL PCB LOS TIENE Y SI LOS TIENE SACARSELOS Y REASIGNARLOS

// 	log_trace(kernel_log_debug, "ENTRE A LIBERAR RECURSOS");
// 	t_list* recursos_del_pcb = list_duplicate(pcb->lista_recursos_pcb);
// 	while(!list_is_empty(recursos_del_pcb)){
// 		//pthread_mutex_lock(&pcb->mutex_lista_recursos);
// 		t_recurso* un_recurso = list_remove(recursos_del_pcb,0);
// 		if(un_recurso != NULL){
// 			pthread_mutex_lock(&un_recurso->mutex_bloqueados);
// 			if(list_remove_element(un_recurso->lista_bloqueados, pcb)){
// 				log_info(kernel_log_debug, "PUDE REMOVER UN UN PCB DE LA LISTA DE BLOQUEADOS DEL RECURSO");
// 				if(list_remove_element(pcb->lista_recursos_pcb, un_recurso)){
// 						log_info(kernel_log_debug, "PUDE REMOVER EL RECURSO DE LA LISTA DE RECURSOS DEL PCB");
// 				}

// 			}else{
// 				un_recurso->instancias++;
// 				// Se manda como un hilo, para que cuando se detiene la planificacion, se pause la asignacion de recursos para los demas PCBs y no entren a READY.
// 				// Tambien agiliza eliminar +1 proceso a la vez.
// 				ejecutar_en_un_hilo_nuevo_detach((void*)asignar_recurso_liberado_pcb, un_recurso);
// 			}
// 			pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
// 		}
// 		//pthread_mutex_unlock(&pcb->mutex_lista_recursos);
// 	}
// 	list_destroy(recursos_del_pcb);
// }


void liberar_recursos_pcb(t_pcb* pcb){

	// TENGO QUE PARA CADA RECURSO SOLICITAOD, VER SI EL PCB LOS TIENE ASIGNADO O SOLO LO SOLICITO
	// SI LO TIENE ASIGNADO TIENE QUE SACARSELO, TIENE QUE SACARSELOS Y REASIGNARLOS
	// SI LO TIENE SOLICITADO, HAY QUE SACAR ESA SOLICITUD

	log_trace(kernel_log_debug, "ENTRE A LIBERAR RECURSOS");
	t_list* recursos_del_pcb = list_duplicate(pcb->lista_recursos_pcb);
	while(!list_is_empty(recursos_del_pcb)){
		//pthread_mutex_lock(&pcb->mutex_lista_recursos);
		t_recurso* un_recurso = list_remove(recursos_del_pcb,0);
		if(un_recurso != NULL){
			pthread_mutex_lock(&un_recurso->mutex_bloqueados);

			// PCB TIENE EL RECURSO

			// INTENTO REMOVER EL PC DE LA LISTA DE BLOQUEADOS
			// SI PUEDO, ES PORQ TENIA EL RECURSO COMO SOLICITADO
			if(list_remove_element(un_recurso->lista_bloqueados, pcb)){

				log_info(kernel_log_debug, "SE CANCELA LA SOLICITUD DEL PROCESO %d PARA EL RECURSO %s", pcb->pid, un_recurso->recurso_name);
				if(list_remove_element(pcb->lista_recursos_pcb, un_recurso)){
						log_info(kernel_log_debug, "PUDE REMOVER EL RECURSO DE LA LISTA DE RECURSOS DEL PCB");
				}

			}else{
				// SI NO PUDE REMOVER EL PROCESO DE LA LISTA DE BLOQUEADOS ES PORQUE EL RECURSO NO ESTA SOLICITADO SINO ASIGNADO
				un_recurso->instancias++;
				// Se manda como un hilo, para que cuando se detiene la planificacion, se pause la asignacion de recursos para los demas PCBs y no entren a READY.
				// Tambien agiliza eliminar +1 proceso a la vez.
				ejecutar_en_un_hilo_nuevo_detach((void*)asignar_recurso_liberado_pcb, un_recurso);
			}
			pthread_mutex_unlock(&un_recurso->mutex_bloqueados);
		}
		//pthread_mutex_unlock(&pcb->mutex_lista_recursos);
	}
	list_destroy(recursos_del_pcb);
}
void agregar_pcb_lista(t_pcb* pcb, t_list* lista_estado, pthread_mutex_t mutex_lista){
	pthread_mutex_lock(&mutex_lista);
	list_add(lista_estado, pcb);
	pthread_mutex_unlock(&mutex_lista);
}

// FUNCIONA
t_pcb* buscar_pcb_por_pid_en(int un_pid, t_list* lista_estado){
	t_pcb* un_pcb;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

	if(list_any_satisfy(lista_estado, (void*)__buscar_pcb)){
		un_pcb = list_find(lista_estado, (void*)__buscar_pcb);
	}
	else{
		un_pcb = NULL;
	}
	return un_pcb;
}

// FUNCIONA
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* un_pcb){
	int var_aux = 0;
	void __buscar_pcb_exacta(t_pcb* void_pcb){
		if(void_pcb == un_pcb){
			var_aux++;
		}
	};

	list_iterate(una_lista, (void*)__buscar_pcb_exacta);
	if(var_aux != 0){
		return true;
	}else{
		return false;
	}
}

// FUNCIONA
t_pcb* obtener_proceso_desalojado(t_contexto* ctxt){
    
	bool buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == ctxt->pID){
			return true;
		} else {
			return false;
		}
	};

	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb* un_pcb = list_find(lista_execute, (void*) buscar_pcb);
	pthread_mutex_unlock(&mutex_lista_exec);

    if(un_pcb==NULL){
        return NULL;
    }

	un_pcb->verificador = ctxt->verificador;
	un_pcb->registros_CPU->PC=ctxt->r_cpu->PC;
    un_pcb->registros_CPU->AX=ctxt->r_cpu->AX;
    un_pcb->registros_CPU->BX=ctxt->r_cpu->BX;
    un_pcb->registros_CPU->CX=ctxt->r_cpu->CX;
    un_pcb->registros_CPU->DX=ctxt->r_cpu->DX;
    un_pcb->registros_CPU->EAX=ctxt->r_cpu->EAX;
    un_pcb->registros_CPU->EBX=ctxt->r_cpu->EBX;
    un_pcb->registros_CPU->ECX=ctxt->r_cpu->ECX;
    un_pcb->registros_CPU->EDX=ctxt->r_cpu->EDX;
    un_pcb->registros_CPU->SI=ctxt->r_cpu->SI;
    un_pcb->registros_CPU->DI=ctxt->r_cpu->DI;

	printf("Contexto de PID: %d\n", un_pcb->pid);
	printf("PC: %d\t", un_pcb->registros_CPU->PC);
	printf("AX: %d\t", un_pcb->registros_CPU->AX);
	printf("BX: %d\t", un_pcb->registros_CPU->BX);
	printf("CX: %d\t", un_pcb->registros_CPU->CX);
	printf("DX: %d\t", un_pcb->registros_CPU->DX);
	printf("EAX: %d\t", un_pcb->registros_CPU->EAX);
	printf("EBX: %d\t", un_pcb->registros_CPU->EBX);
	printf("ECX: %d\t", un_pcb->registros_CPU->ECX);
	printf("EDX: %d\t", un_pcb->registros_CPU->EDX);
	printf("SI: %d\t", un_pcb->registros_CPU->SI);
	printf(" DI: %d\n", un_pcb->registros_CPU->DI);

	return un_pcb;
}

// t_mochila* copiar_mochila(t_mochila* original) {
//     t_mochila* copia = malloc(sizeof(t_mochila));
//     if (copia == NULL) {
//         return NULL;
//     }

//     // Duplicate instruccionAsociada
//     copia->instruccionAsociada = strdup(original->instruccionAsociada);
//     if (copia->instruccionAsociada == NULL) {
//         free(copia);
//         return NULL;
//     }

//     copia->cantidad_parametros_inicial = original->cantidad_parametros_inicial;

//     // Create new queue for parametros
//     copia->parametros = queue_create();
//     if (copia->parametros == NULL) {
//         free(copia->instruccionAsociada);
//         free(copia);
//         return NULL;
//     }

//     // Temporary list to store elements while iterating
//     t_list* temp_list = list_create();
//     if (temp_list == NULL) {
//         free(copia->instruccionAsociada);
//         queue_destroy(copia->parametros);
//         free(copia);
//         return NULL;
//     }

//     // Iterate over the original queue and duplicate each element
//     while (!queue_is_empty(original->parametros)) {
//         void* elemento = queue_pop(original->parametros);
//         if (elemento != NULL) {
//             // Assuming we can use a custom duplicate function for each element
//             void* elemento_duplicado = duplicate_element(elemento); // Implement this based on element type
//             if (elemento_duplicado == NULL) {
//                 // Clean up memory in case of error
//                 free(copia->instruccionAsociada);
//                 queue_destroy_and_destroy_elements(copia->parametros, free);
//                 list_add(temp_list, elemento);
//                 for (int i = 0; i < list_size(temp_list); i++) {
//                     queue_push(original->parametros, list_get(temp_list, i));
//                 }
//                 list_destroy(temp_list);
//                 free(copia);
//                 return NULL;
//             }

//             queue_push(copia->parametros, elemento_duplicado);
//             list_add(temp_list, elemento);  // Save the element in the temporary list
//         }
//     }

//     // Return the elements to the original queue
//     for (int i = 0; i < list_size(temp_list); i++) {
//         queue_push(original->parametros, list_get(temp_list, i));
//     }

//     // Clean up the temporary list
//     list_destroy(temp_list);

//     return copia;
// }

// void* duplicate_element(void* element) {
//     // Assuming the elements can be duplicated with memcpy or a similar method
//     // Adjust this implementation based on the actual type of elements
//     void* new_element = malloc(sizeof(*element));
//     if (new_element == NULL) {
//         return NULL;
//     }
//     memcpy(new_element, element, sizeof(*element));
//     return new_element;
// }


t_mochila* copiar_mochila(t_mochila* original) {
    t_mochila* copia = malloc(sizeof(t_mochila));
    if (copia == NULL) {
        return NULL;
    }

    // Duplicar instruccionAsociada
    copia->instruccionAsociada = strdup(original->instruccionAsociada);
    if (copia->instruccionAsociada == NULL) {
        free(copia);
        return NULL;
    }

    copia->cantidad_parametros_inicial = original->cantidad_parametros_inicial;

	copia->parametros = queue_create();

    // Crear nueva cola para parametros
    copia->parametros->elements = list_duplicate(original->parametros->elements);
    if (copia->parametros == NULL) {

		if(copia->instruccionAsociada != NULL){
			free(copia->instruccionAsociada);
			copia->instruccionAsociada = NULL;
		}
        if(copia != NULL){

        	free(copia);
			copia = NULL;
		}
        return NULL;
    }

    return copia;
}

// t_mochila* copiar_mochila(t_mochila* original) {
//     t_mochila* copia = malloc(sizeof(t_mochila));
//     if (copia == NULL) {
//         return NULL;
//     }

//     // Duplicar instruccionAsociada
//     copia->instruccionAsociada = strdup(original->instruccionAsociada);
//     if (copia->instruccionAsociada == NULL) {
//         free(copia);
//         return NULL;
//     }

//     copia->cantidad_parametros_inicial = original->cantidad_parametros_inicial;

//     // Crear nueva cola para parametros
//     copia->parametros = queue_create();
//     if (copia->parametros == NULL) {
//         free(copia->instruccionAsociada);
//         free(copia);
//         return NULL;
//     }

//     // Lista temporal para almacenar elementos mientras iteramos
//     t_list* temp_list = list_create();

//     // Iterar sobre la cola original y duplicar cada elemento
//     while (!queue_is_empty(original->parametros)) {
//         void* elemento = queue_pop(original->parametros);
//         if (elemento != NULL) {
//             // Asumiendo que los elementos de la cola son de tipo int*
//             int* elemento_duplicado = malloc(sizeof(int));
//             if (elemento_duplicado == NULL) {
//                 // Limpiar memoria en caso de error
//                 free(copia->instruccionAsociada);
//                 queue_destroy_and_destroy_elements(copia->parametros, free);
//                 free(copia);
//                 list_destroy(temp_list);
//                 return NULL;
//             }

//             *elemento_duplicado = *(int*)elemento;
//             queue_push(copia->parametros, elemento_duplicado);
//             list_add(temp_list, elemento);  // Guardar el elemento en la lista temporal
//         }
//     }

//     // Devolver los elementos a la cola original
//     for (int i = 0; i < list_size(temp_list); i++) {
//         queue_push(original->parametros, list_get(temp_list, i));
//     }

//     // Limpiar la lista temporal
//     list_destroy(temp_list);

//     return copia;
// }

// t_pcb* duplicar_pcb(t_pcb* original) {
//     if (original == NULL) {
//         return NULL;
//     }

//     t_pcb* copia = malloc(sizeof(t_pcb));
//     if (copia == NULL) {
//         return NULL;
//     }

//     copia->pid = original->pid;
//     copia->quantum = original->quantum;

//     // Duplicar registros_CPU
//     if (original->registros_CPU != NULL) {
//         copia->registros_CPU = malloc(sizeof(t_registrosCPU));
//         if (copia->registros_CPU == NULL) {
//             free(copia);
//             return NULL;
//         }
//         memcpy(copia->registros_CPU, original->registros_CPU, sizeof(t_registrosCPU));
//     } else {
//         copia->registros_CPU = NULL;
//     }

//     copia->verificador = original->verificador;

//     // Duplicar path
//     if (original->path != NULL) {
//         copia->path = strdup(original->path);
//         if (copia->path == NULL) {
//             free(copia->registros_CPU);
//             free(copia);
//             return NULL;
//         }
//     } else {
//         copia->path = NULL;
//     }

//     copia->estado = original->estado;

//     // Duplicar lista_recursos_pcb
//     if (original->lista_recursos_pcb != NULL) {
//         copia->lista_recursos_pcb = list_create(); // Supone que tienes una función list_create() que inicializa una lista vacía
//         if (copia->lista_recursos_pcb == NULL) {
//             free(copia->path);
//             free(copia->registros_CPU);
//             free(copia);
//             return NULL;
//         }

//         // Aquí debes duplicar cada elemento de la lista. Esto depende de lo que haya en la lista.
//         // Supone que hay una función list_size() que devuelve el tamaño de la lista
//         // y list_get() que obtiene un elemento en una posición específica.
//         for (int i = 0; i < list_size(original->lista_recursos_pcb); i++) {
//             void* recurso_original = list_get(original->lista_recursos_pcb, i);
//             // Duplicar recurso_original según su tipo y agregarlo a copia->lista_recursos_pcb.
//             // Aquí debes implementar la lógica específica para duplicar cada tipo de recurso.
//             void* recurso_copia = duplicar_recurso(recurso_original); // Supone que tienes una función duplicar_recurso()
//             if (recurso_copia == NULL) {
//                 // Liberar toda la memoria asignada previamente
//                 free(copia->lista_recursos_pcb);
//                 free(copia->path);
//                 free(copia->registros_CPU);
//                 free(copia);
//                 return NULL;
//             }
//             list_add(copia->lista_recursos_pcb, recurso_copia); // Supone que tienes una función list_add()
//         }
//     } else {
//         copia->lista_recursos_pcb = NULL;
//     }

//     copia->motivo_exit = original->motivo_exit;

//     // Duplicar timer
//     if (original->timer != NULL) {
//         copia->timer = malloc(sizeof(t_temporal));
//         if (copia->timer == NULL) {
//             // Liberar toda la memoria asignada previamente
//             free(copia->lista_recursos_pcb);
//             free(copia->path);
//             free(copia->registros_CPU);
//             free(copia);
//             return NULL;
//         }
//         memcpy(copia->timer, original->timer, sizeof(t_temporal));
//     } else {
//         copia->timer = NULL;
//     }

//     copia->flag_proceso_venia_de_ready_prioridad = original->flag_proceso_venia_de_ready_prioridad;
//     copia->flag_proceso_desalojado_por_prioridad = original->flag_proceso_desalojado_por_prioridad;
//     copia->flag_proceso_desalojado = original->flag_proceso_desalojado;
//     copia->flag_finalizar_proceso = original->flag_finalizar_proceso;
// 	copia->flag_cancelar_quantum = original->flag_cancelar_quantum;

//     return copia;
// }

t_pcb* duplicar_pcb(t_pcb* original) {
    if (original == NULL) {
        return NULL;
    }

    t_pcb* copia = malloc(sizeof(t_pcb));
    if (copia == NULL) {
        return NULL;
    }

    copia->pid = original->pid;
    copia->quantum = original->quantum;

    // Duplicar registros_CPU
    if (original->registros_CPU != NULL) {
        copia->registros_CPU = malloc(sizeof(t_registrosCPU));
        if (copia->registros_CPU == NULL) {

			if(copia != NULL){

            	free(copia);
				copia = NULL;
			}
            return NULL;
        }
        memcpy(copia->registros_CPU, original->registros_CPU, sizeof(t_registrosCPU));
    } else {
        copia->registros_CPU = NULL;
    }

    copia->verificador = original->verificador;

    // Duplicar path
    if (original->path != NULL) {
        copia->path = strdup(original->path);
        if (copia->path == NULL) {

			if(copia->registros_CPU != NULL){

            	free(copia->registros_CPU);
				copia->registros_CPU = NULL;

			}

            if(copia != NULL){

            	free(copia);
				copia = NULL;
			}
            return NULL;
        }
    } else {
        copia->path = NULL;
    }

    copia->estado = original->estado;

	

    // Duplicar lista_recursos_pcb
    if (original->lista_recursos_pcb != NULL) {
        copia->lista_recursos_pcb = list_create(); // Supone que tienes una función list_create() que inicializa una lista vacía
        if (copia->lista_recursos_pcb == NULL) {

			if(copia->path != NULL){

            	free(copia->path);
				copia->path = NULL;
			}
            if(copia->registros_CPU != NULL){

            	free(copia->registros_CPU);
				copia->registros_CPU = NULL;

			}
            if(copia != NULL){

            	free(copia);
				copia = NULL;
			}
            return NULL;
        }

		copia->lista_recursos_pcb = list_duplicate(original->lista_recursos_pcb);
        
    } else {
        copia->lista_recursos_pcb = NULL;
    }

    copia->motivo_exit = original->motivo_exit;

    // Duplicar timer
    if (original->timer != NULL) {
        copia->timer = malloc(sizeof(t_temporal));
        if (copia->timer == NULL) {
            // Liberar toda la memoria asignada previamente

			if(copia->lista_recursos_pcb != NULL){

            	free(copia->lista_recursos_pcb);
				copia->lista_recursos_pcb = NULL;
			}

			if(copia->path != NULL){

            	free(copia->path);
				copia->path = NULL;
			}

			if(copia->registros_CPU != NULL){

            	free(copia->registros_CPU);
				copia->registros_CPU = NULL;
			}
			
            if(copia != NULL){

            	free(copia);
				copia = NULL;
			}
            return NULL;
        }
        memcpy(copia->timer, original->timer, sizeof(t_temporal));
    } else {
        copia->timer = NULL;
    }

    copia->flag_proceso_venia_de_ready_prioridad = original->flag_proceso_venia_de_ready_prioridad;
    copia->flag_proceso_desalojado_por_prioridad = original->flag_proceso_desalojado_por_prioridad;
    copia->flag_proceso_desalojado = original->flag_proceso_desalojado;
    copia->flag_finalizar_proceso = original->flag_finalizar_proceso;
	copia->flag_cancelar_quantum = original->flag_cancelar_quantum;

    return copia;
}


// Ejemplo de función duplicar_recurso (debes implementarla según el tipo de recursos en la lista)
void* duplicar_recurso(void* recurso_original) {
    // Aquí debes implementar la lógica específica para duplicar cada tipo de recurso.
    // Este es un ejemplo genérico.
    void* recurso_copia = malloc(sizeof(t_recurso));
    if (recurso_copia == NULL) {
        return NULL;
    }
    memcpy(recurso_copia, recurso_original, sizeof(t_recurso));
    return recurso_copia;
}