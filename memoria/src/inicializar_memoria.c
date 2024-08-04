#include "../include/inicializar_memoria.h"
#include "../include/marcos.h"

void inicializar_memoria(char* archivo_config){

	inicializar_logs();
	inicializar_configs(archivo_config);
	inicializar_pthread();
	inicializar_estructuras();
	sem_init(&sem_finalizo_kernel, 0, 0);
}

void inicializar_logs(){
	memoria_log_debug = log_create("Memoria.log", "[Memoria]",1,LOG_LEVEL_TRACE);

	if(memoria_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	memoria_log_obligatorio = log_create("Memoria_log_obligatorio.log", "[Memoria- Log obligatorio]",1,LOG_LEVEL_INFO);

	if(memoria_log_obligatorio == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

}

void inicializar_configs(char* archivo_config){

	if((memoria_config = config_create(archivo_config)) == NULL)
	{
		printf("Error al crear el archivo de configuracion");
		exit(2);
	}

	PUERTO_ESCUCHA  = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
	TAM_MEMORIA = config_get_int_value(memoria_config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(memoria_config, "TAM_PAGINA");
	PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(memoria_config, "RETARDO_RESPUESTA");
}

void inicializar_pthread(){
	pthread_mutex_init(&mutex_espacio_usuario, NULL);
	pthread_mutex_init(&mutex_lst_instancias_io, NULL);
	pthread_mutex_init(&mutex_lst_marco, NULL);
	pthread_mutex_init(&mutex_lst_procss_recibidos, NULL);
}

void inicializar_estructuras(){

	espacio_usuario = malloc(TAM_MEMORIA);
	
	if(espacio_usuario == NULL){
			log_error(memoria_log_debug, "Fallo Malloc");
	    	exit(1);
	    }

	list_procss_recibidos = list_create();
	lst_marco = list_create();
	lista_instancias_io = list_create();
	int cant_marcos = TAM_MEMORIA/TAM_PAGINA;

	for(int i=0;i< cant_marcos;i++){
		t_marco* nuevo_marco  = crear_marco(TAM_PAGINA*i, true, i);

		pthread_mutex_lock(&mutex_lst_marco);
		list_add(lst_marco,nuevo_marco);
		pthread_mutex_unlock(&mutex_lst_marco);
	}
}