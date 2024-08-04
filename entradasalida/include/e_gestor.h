#ifndef E_GESTOR_H_
#define E_GESTOR_H_

#include <pthread.h>
#include <semaphore.h>
#include "../../utils/include/shared.h"  

typedef struct{
	char* nombre;
	size_t tamanio;
    int tamanio_en_bloques;
	int bloque_inicial;
	t_config * archivo_metadata;
}t_fcb;

// Declaraciones de variables globales
extern bool terminar;
extern t_log* io_log_debug;
extern t_log* io_log_obligatorio;
extern t_config* io_config;
extern int fd_kernel;
extern int fd_memoria;

// Semáforos para sincronización
extern sem_t sem_kernel;
extern sem_t sem_escribio_memoria;
extern sem_t sem_leyo_data;
extern sem_t sem_proceso_en_io;

extern char* NOMBRE_INTERFAZ;
extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL; 
extern char* PUERTO_KERNEL; 
extern char* IP_MEMORIA; 
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE; 
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;

extern int TIPO_INTERFAZ_ENUM;

extern int TAM_PAGINA;

extern char* data_leida;

// --------------------------------------------------

extern char* PATH_ARCHIVO_BLOQUES;
extern char* PATH_BITMAP;

extern t_list* lista_fat;

extern int fd_archivoBloques;
extern int fd_bitmap;

extern char* bitmapChar;
extern t_bitarray* bitmap;

extern void* bloquesEnMemoria;

extern pthread_mutex_t mutex_bloquesEnMemoria; 
extern pthread_mutex_t mutex_bitmap; 
extern pthread_mutex_t mutex_lista_fat; 


#endif /* E_GESTOR_H_ */
