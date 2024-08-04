#include "../include/servicios_kernel.h"
#include "../include/pcb.h"

void pausador(){
	pthread_mutex_lock(&mutex_pausa);
	if(var_pausa == 1){
		
		// pthread_mutex_lock(&mutex_lista_exec);
		// t_pcb* pcbEnExec = list_get(lista_execute,0);
		// pthread_mutex_unlock(&mutex_lista_exec);

		// if(pcbEnExec->timer != NULL)
		// 	temporal_stop(pcbEnExec->timer);

		log_info(kernel_log_debug, "PAUSA DE PLANIFICACIÓN");

		sem_wait(&sem_pausa);

		// if(pcbEnExec->timer != NULL)
		// 	temporal_resume(pcbEnExec->timer);

	}
	pthread_mutex_unlock(&mutex_pausa);
}

int generar_verificador(){
	int valor_verificador;
	pthread_mutex_lock(&mutex_verificador);
	var_verificador++;
	valor_verificador = var_verificador;
	pthread_mutex_unlock(&mutex_verificador);
	return valor_verificador;
}

char* algoritmo_to_string(t_algoritmo algoritmo){

	switch(algoritmo){
	case FIFO:
		return "FIFO";
		break;
	case ROUNDROBIN:
		return "RR";
		break;
	case VIRTUALROUNDROBIN:
		return "VRR";
		break;
	default:
		return "ERROR";
	}
}

// ----- LOG PROCESOS BLOQUEADOS -----
void log_blocked_proceso(int pid_process, char* motivo_block){
	log_info(kernel_log_obligatorio, "PID: %d - Bloqueado por: %s", pid_process, motivo_block);
}

float nuevo_quantum(t_temporal* timer){
	return (QUANTUM - (float) temporal_gettime(timer));
}

bool debo_asignar_nuevo_quantum(t_temporal* timer){
	if(timer != NULL)
		return (QUANTUM - (float) temporal_gettime(timer)) > 0;
	else{
		return 0;
	}
}

void asignoQuantumEnVRR(t_pcb* un_pcb){

	if(!(un_pcb->flag_proceso_venia_de_ready_prioridad) && debo_asignar_nuevo_quantum(un_pcb->timer)){
		un_pcb->quantum = nuevo_quantum(un_pcb->timer);
		transferir_from_actual_to_siguiente(un_pcb, lista_ready_prioridad, mutex_lista_ready_prioridad, READY_PRIORIDAD);
	}
	else
	{
		un_pcb->quantum = QUANTUM;
		transferir_from_actual_to_siguiente(un_pcb, lista_ready, mutex_lista_ready, READY);
	}

	if(un_pcb->timer != NULL){
		temporal_destroy(un_pcb->timer);
		un_pcb->timer = NULL;
	}
}

void analizarSiLoMandoAReadyOReadyPrioridad(t_pcb* un_pcb){

	pausador();

	if((ALGORITMO_PLANIFICACION == ROUNDROBIN) || (ALGORITMO_PLANIFICACION == FIFO)){
		transferir_from_actual_to_siguiente(un_pcb, lista_ready, mutex_lista_ready, READY);
    }
    else if (ALGORITMO_PLANIFICACION == VIRTUALROUNDROBIN){
        asignoQuantumEnVRR(un_pcb);
    }
}

bool proceso_ejecutara_io(char * instruccion){
	return (instruccion[0] == 'I' && instruccion[1] == 'O');
}

t_mochila* crear_mochila() {
    t_mochila* mochila = malloc(sizeof(t_mochila));

    if (mochila == NULL) return NULL;

    mochila->parametros = queue_create();
    mochila->instruccionAsociada = NULL;
	mochila->cantidad_parametros_inicial = 0;

    return mochila;
}