#include "../include/inicializar_kernel.h"

void inicializar_kernel(char* archivo_config) {

	inicializar_logs();
	inicializar_configs(archivo_config);
	iniciar_listas();
	iniciar_semaforos();
	iniciar_pthread();
	_iniciar_recursos();
}

void inicializar_logs(){

	kernel_log_debug = log_create("kernel_debug.log","KERNEL_DEBUG_LOG",1,LOG_LEVEL_TRACE);

	if(kernel_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	kernel_log_obligatorio = log_create("kernel_log_obligatorio.log", "[Kernel - Log obligatorio]", 1, LOG_LEVEL_INFO);
}

void inicializar_configs(char* archivo_config){
	if((kernel_config = config_create(archivo_config)) == NULL)
	{
		printf("Error al crear el archivo de configuracion");
		exit(2);
	}

	PUERTO_ESCUCHA = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
	// IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
	// IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
	algoritmo_planificacion = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
	QUANTUM = (float) config_get_int_value(kernel_config, "QUANTUM");
	RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
	INSTANCIAS_RECURSOS_EN_CARACTERES = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS");
	GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");

	// INSTANCIAS_RECURSOS = convertirCharAInt(INSTANCIAS_RECURSOS_EN_CARACTERES);

	if(strcmp(algoritmo_planificacion, "FIFO") == 0) {
			ALGORITMO_PLANIFICACION = FIFO;
	} else if (strcmp(algoritmo_planificacion, "RR") == 0) {
		ALGORITMO_PLANIFICACION = ROUNDROBIN;
	} else if (strcmp(algoritmo_planificacion, "VRR") == 0) {
		ALGORITMO_PLANIFICACION = VIRTUALROUNDROBIN;
	} else {
		log_error(kernel_log_debug, "No se encontro el algoritmo de planificacion de corto plazo");
	}
}

void iniciar_semaforos(){
	sem_init(&sem_pausa, 0, 0);
	sem_init(&sem_enviar_interrupcion_consola, 0, 0);
	sem_init(&sem_enviar_interrupcion_prioridad, 0, 0);
	sem_init(&sem_enviar_interrupcion_quantum, 0, 0);
	sem_init(&sem_estructura_iniciada, 0, 0);
	sem_init(&sem_estructura_liberada, 0, 0);
	sem_init(&sem_procesos_en_memoria, 0, GRADO_MULTIPROGRAMACION);
	// sem_init(&sem_nuevo_en_block, 0, 0);
}


void iniciar_listas(){
	lista_new = list_create();
	lista_ready = list_create();
	lista_execute = list_create();
	lista_blocked = list_create();
	lista_exit = list_create();
	lista_ready_prioridad = list_create();

	lista_recursos = list_create();

	lista_instancias_global = list_create();
}

void iniciar_pthread(){
	pthread_mutex_init(&mutex_lista_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_lista_exit, NULL);
	pthread_mutex_init(&mutex_lista_ready_prioridad, NULL);


	pthread_mutex_init(&mutex_process_id, NULL);
	pthread_mutex_init(&mutex_pausa, NULL);
	pthread_mutex_init(&mutex_verificador, NULL);

	pthread_mutex_init(&mutex_lista_instancias_global, NULL);

	pthread_mutex_init(&mutex_lista_recursos, NULL);
	pthread_mutex_init(&mutex_lista_instrucciones, NULL);
}

void _iniciar_recursos(){
	int i = 0;
	while(RECURSOS[i] != NULL){
		t_recurso* recurso = malloc(sizeof(t_recurso));
		recurso->recurso_name = RECURSOS[i];
		recurso->instancias = atoi(INSTANCIAS_RECURSOS_EN_CARACTERES[i]);
		recurso->pcb_asignado = NULL;
		recurso->lista_bloqueados = list_create();
		list_add(lista_recursos, recurso);
		i++;

		pthread_mutex_init(&recurso->mutex_bloqueados, NULL);
	}
}