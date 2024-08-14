#include "../include/memoria.h"

int main(int argc, char* argv[]) {

	char* archivo_config = argv[1];
	inicializar_memoria(archivo_config);

	fd_memoria = iniciar_servidor(PUERTO_ESCUCHA, memoria_log_debug, "MEMORIA INICIADA");

	log_info(memoria_log_debug, "ESPERANDO A CPU");
	fd_cpu = esperar_cliente(fd_memoria, memoria_log_debug, "CPU");

	// // ATENDER LOS MENSAJES DE CPU
	pthread_t hilo_cpu;
	pthread_create(&hilo_cpu, NULL, (void*) atender_memoria_cpu, NULL);
	pthread_detach(hilo_cpu);

	log_info(memoria_log_debug, "ESPERANDO A KERNEL");
	fd_kernel = esperar_cliente(fd_memoria, memoria_log_debug, "KERNEL");

	// ATENDER LOS MENSAJES DE KERNEL
	pthread_t hilo_kernel;
	pthread_create(&hilo_kernel, NULL, (void*) atender_memoria_kernel, NULL);
	pthread_detach(hilo_kernel);

	pthread_t hilo_escuchar_ES;
	pthread_create(&hilo_escuchar_ES, NULL, (void*) escuchar_ES, NULL);
	pthread_detach(hilo_escuchar_ES);


	sem_wait(&sem_finalizo_kernel);

	// FINALIZAR CONSOLA
	finalizar_memoria();

	printf("TODO MEMORIA SE FINALIZO CORRECTAMENTE...\n");
	return EXIT_SUCCESS;
}

void escuchar_ES(){
    while(server_escucha_entrada_salida());
}

int server_escucha_entrada_salida(){
	log_info(memoria_log_debug, "ESCUCHANDO ENTRADA SALIDA");

	int fd_entradasalida = esperar_cliente(fd_memoria, memoria_log_debug, "ENTRADA SALIDA");
	if(fd_entradasalida != -1){

		t_instancia_io* nueva_instancia = malloc(sizeof(t_instancia_io));
		nueva_instancia->socket = fd_entradasalida;

		pthread_mutex_lock(&mutex_lst_instancias_io);
		list_add(lista_instancias_io, nueva_instancia);
		pthread_mutex_unlock(&mutex_lst_instancias_io);

		pthread_t hilo_entradasalida;
		pthread_create(&hilo_entradasalida, NULL, (void*) atender_memoria_entradasalida, (void *) nueva_instancia); 
		pthread_detach(hilo_entradasalida);

		return 1;
	}

	return EXIT_SUCCESS;
}

void destruir_interfaz_socket(t_instancia_io* instancia){
	liberar_conexion(instancia->socket);

	if(instancia != NULL){
		free(instancia);
		instancia = NULL;
	}
}

void finalizar_memoria(){

	destruir_list_marcos_y_todos_los_marcos();

	if(espacio_usuario != NULL){
		free(espacio_usuario);
		espacio_usuario = NULL;
	}

	list_destroy_and_destroy_elements(list_procss_recibidos, (void*)eliminar_proceso);

	log_destroy(memoria_log_debug);
	log_destroy(memoria_log_obligatorio);
	config_destroy(memoria_config);

	pthread_mutex_destroy(&mutex_espacio_usuario);
	pthread_mutex_destroy(&mutex_lst_instancias_io);
	pthread_mutex_destroy(&mutex_lst_marco);
	pthread_mutex_destroy(&mutex_lst_procss_recibidos);

	list_destroy_and_destroy_elements(lista_instancias_io, (void*)destruir_interfaz_socket);
	
	liberar_conexion(fd_cpu);
	liberar_conexion(fd_kernel);

	sem_destroy(&sem_finalizo_kernel);
}

