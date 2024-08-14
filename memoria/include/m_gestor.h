#ifndef M_GESTOR_H_
#define M_GESTOR_H_

// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
#include "../../utils/include/shared.h"

typedef struct{
	int pid;
	char* pathInstrucciones;
	t_list* instrucciones;
	t_list* tabla_paginas;
	pthread_mutex_t mutex_TP; 
}t_proceso;

typedef struct {
	t_proceso* proceso;
	int nro_pagina;
}frame_info;

typedef struct {
    int nro_marco;
    int base;
    bool libre;
    frame_info* info_new;
    frame_info* info_old;
} t_marco;

typedef struct {
	int nro_pagina; 
	int nro_marco;
} t_pagina;

typedef struct {
    int socket;
} t_instancia_io;

// VARIABLES GLOBALES
extern t_log* memoria_log_debug;
extern t_log* memoria_log_obligatorio;
extern t_config* memoria_config;

extern int fd_memoria;
extern int fd_cpu;
extern int fd_kernel;

extern char* PUERTO_ESCUCHA;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;

extern t_list* lst_marco;

extern void* espacio_usuario;

extern t_list* list_procss_recibidos;
extern t_list* lista_instancias_io;

extern pthread_mutex_t mutex_lst_marco;
extern pthread_mutex_t mutex_espacio_usuario;
extern pthread_mutex_t mutex_lst_instancias_io;
extern pthread_mutex_t mutex_lst_procss_recibidos;

extern sem_t sem_finalizo_kernel;


#endif /* M_GESTOR_H_ */
