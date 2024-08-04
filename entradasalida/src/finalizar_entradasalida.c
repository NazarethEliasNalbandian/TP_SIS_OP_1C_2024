#include "../include/finalizar_entradasalida.h"
#include "../include/fcb.h"

void finalizar_entradasalida()
{
    finalizar_logs();
    finalizar_configs();
    finalizar_semaforos();
	finalizar_conexiones();
	if(TIPO_INTERFAZ_ENUM == DIALFS)
	{
		finalizar_estructuras_filesystem();
	}
}

void finalizar_logs(){
    log_destroy(io_log_obligatorio);
	log_destroy(io_log_debug);
}

void finalizar_configs(){
	config_destroy(io_config);
}

void finalizar_semaforos(){
	sem_destroy(&sem_escribio_memoria);
	sem_destroy(&sem_kernel);
	sem_destroy(&sem_leyo_data);
	sem_destroy(&sem_proceso_en_io);
}

void finalizar_conexiones(){
	liberar_conexion(fd_kernel);
	liberar_conexion(fd_memoria);
}

void finalizar_estructuras_filesystem(){
	destruir_listas_fcbs();

	close(fd_bitmap);
	close(fd_archivoBloques);

	bitarray_destroy(bitmap);

	if(PATH_BITMAP != NULL){

		free(PATH_BITMAP);
		PATH_BITMAP = NULL;
	}

	if(PATH_ARCHIVO_BLOQUES != NULL){

		free(PATH_ARCHIVO_BLOQUES);
		PATH_ARCHIVO_BLOQUES = NULL;
	}

	if(data_leida != NULL)
	{
		free(data_leida);
		data_leida = NULL;
	}

	pthread_mutex_destroy(&mutex_bitmap);
	pthread_mutex_destroy(&mutex_lista_fat);
	pthread_mutex_destroy(&mutex_bloquesEnMemoria);
}