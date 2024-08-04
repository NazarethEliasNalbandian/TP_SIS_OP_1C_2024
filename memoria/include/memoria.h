#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "m_gestor.h"
#include "inicializar_memoria.h"
#include "memoria_entradasalida.h"
#include "memoria_cpu.h"
#include "memoria_kernel.h"
#include "proceso.h"

void sigint_handler(int sig); 
int server_escucha_entrada_salida();
void destruir_interfaz_socket(t_instancia_io* instancia);
void finalizar_memoria();
void escuchar_ES();

// VARIABLES GLOBALES
t_log* memoria_log_debug;
t_log* memoria_log_obligatorio;
t_config* memoria_config;

int fd_memoria;
int fd_cpu;
int fd_kernel;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;

t_list* list_procss_recibidos;
void* espacio_usuario;
t_list* lst_marco;
t_list* lista_instancias_io;

pthread_mutex_t mutex_lst_marco;
pthread_mutex_t mutex_espacio_usuario;
pthread_mutex_t mutex_lst_instancias_io;
pthread_mutex_t mutex_lst_procss_recibidos;

sem_t sem_finalizo_kernel;

#endif /* MEMORIA_H_ */
