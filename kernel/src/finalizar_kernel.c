#include "../include/finalizar_kernel.h"
#include "../include/entrada_salida.h"
#include "../include/pcb.h"

void finalizar_kernel(){
	_finalizar_config();
	_eliminar_interfases();
	_eliminar_pcbs();
	_finalizar_recursos();
	_destruir_conexiones();
	_finalizar_listas();
	_finalizar_semaforos();
	_finalizar_pthread();
	_finalizar_logger();
}

void _finalizar_logger(){
	log_destroy(kernel_log_obligatorio);
	log_destroy(kernel_log_debug);
    
}

void _finalizar_config(){
	string_array_destroy(RECURSOS);
	string_array_destroy(INSTANCIAS_RECURSOS_EN_CARACTERES);
	config_destroy(kernel_config);
}

void _finalizar_listas() {
    if (lista_new != NULL) {
        list_destroy(lista_new);
        lista_new = NULL;
    }
    if (lista_ready != NULL) {
        list_destroy(lista_ready);
        lista_ready = NULL;
    }
    if (lista_execute != NULL) {
        list_destroy(lista_execute);
        lista_execute = NULL;
    }
    if (lista_blocked != NULL) {
        list_destroy(lista_blocked);
        lista_blocked = NULL;
    }
    if (lista_exit != NULL) {
        list_destroy(lista_exit);
        lista_exit = NULL;
    }
    if (lista_ready_prioridad != NULL) {
        list_destroy(lista_ready_prioridad);
        lista_ready_prioridad = NULL;
    }

    if (lista_instancias_global != NULL) {
        list_destroy(lista_instancias_global);
        lista_instancias_global = NULL;
    }
}


void _finalizar_semaforos(){
	sem_destroy(&sem_pausa);
	sem_destroy(&sem_enviar_interrupcion_consola);
	sem_destroy(&sem_enviar_interrupcion_prioridad);
	sem_destroy(&sem_enviar_interrupcion_quantum);
	sem_destroy(&sem_estructura_iniciada);
	sem_destroy(&sem_estructura_liberada);
	sem_destroy(&sem_procesos_en_memoria);
}

void _finalizar_pthread(){
	pthread_mutex_destroy(&mutex_lista_new);
	pthread_mutex_destroy(&mutex_lista_ready);
	pthread_mutex_destroy(&mutex_lista_exec);
	pthread_mutex_destroy(&mutex_lista_blocked);
	pthread_mutex_destroy(&mutex_lista_exit);

	pthread_mutex_destroy(&mutex_process_id);
	pthread_mutex_destroy(&mutex_pausa);
	pthread_mutex_destroy(&mutex_verificador);

	pthread_mutex_destroy(&mutex_lista_instancias_global);

    pthread_mutex_destroy(&mutex_lista_recursos);
	pthread_mutex_destroy(&mutex_lista_instrucciones);
}

void __eliminar_nodo_recurso(t_recurso* un_recurso) {
	list_destroy(un_recurso->lista_bloqueados);
	pthread_mutex_destroy(&un_recurso->mutex_bloqueados);
	// Procesos con instancias asignadas
	un_recurso->pcb_asignado = NULL;
    if(un_recurso != NULL){

	    free(un_recurso);
        un_recurso = NULL;
    }
}

void _finalizar_recursos(){
	if (lista_recursos != NULL) {
		list_destroy_and_destroy_elements(lista_recursos, (void*)__eliminar_nodo_recurso);
	}
}

void _destruir_conexiones(){
	liberar_conexion(fd_cpu_dispatch);
	liberar_conexion(fd_cpu_interrupt);
	liberar_conexion(fd_memoria);
}

void _eliminar_interfases(){
    // Liberar todas las interfaces en cada lista
    if (lista_instancias_global != NULL) {
        // list_clean_and_destroy_elements(lista_instancias_global, (void*)destruir_interfaz);
    }
}

void _eliminar_pcbs(){
    // Liberar todos los PCBs en cada lista
    if (lista_new != NULL) {
        list_clean_and_destroy_elements(lista_new, (void*)destruir_pcb);
    }
    if (lista_ready != NULL) {
        list_clean_and_destroy_elements(lista_ready, (void*)destruir_pcb);
    }
    if (lista_execute != NULL) {
        // list_clean_and_destroy_elements(lista_execute, (void*)destruir_pcb);
    }
    if (lista_blocked != NULL) {
        list_clean_and_destroy_elements(lista_blocked, (void*)destruir_pcb);
    }
    if (lista_exit != NULL) {
        list_clean_and_destroy_elements(lista_exit, (void*)destruir_pcb);
    }
    if (lista_ready_prioridad != NULL) {
        list_clean_and_destroy_elements(lista_ready_prioridad, (void*)destruir_pcb);
    }
}
