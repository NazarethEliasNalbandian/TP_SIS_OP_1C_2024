#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include "e_gestor.h"

// Definición de variables globales
t_log* io_log_debug;
t_log* io_log_obligatorio;
t_config* io_config;
int fd_kernel;
int fd_memoria;

char* NOMBRE_INTERFAZ;
char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL; 
char* PUERTO_KERNEL; 
char* IP_MEMORIA; 
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE; 
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

int TIPO_INTERFAZ_ENUM;

int TAM_PAGINA;

// Semáforos para sincronización
sem_t sem_kernel;
sem_t sem_escribio_memoria;
sem_t sem_leyo_data;
sem_t sem_proceso_en_io;

char* data_leida;

// --------------------------------------------------------

char* PATH_ARCHIVO_BLOQUES;
char* PATH_BITMAP;

int tamanio_fat;
t_list* lista_fat;
t_list* lista_configs_fcbs;

int fd_archivoTablaFAT;
int fd_archivoBloques;
int fd_bitmap;

char* bitmapChar;
t_bitarray* bitmap;

void* bloquesEnMemoria;

pthread_mutex_t mutex_bloquesEnMemoria; 
pthread_mutex_t mutex_lista_fat; 
pthread_mutex_t mutex_bitmap; 

#endif /* ENTRADASALIDA_H_ */
