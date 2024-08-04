#ifndef KERNEL_H_
#define KERNEL_H_

#include "k_gestor.h"
#include "inicializar_kernel.h"
#include "kernel_memoria.h"
#include "kernel_cpu_dispatch.h"
#include "kernel_cpu_interrupt.h"
#include "kernel_entradasalida.h"
#include "pcb.h"
#include "consola.h"

// VARIABLES GLOBALES
t_log* kernel_log_debug;
t_log* kernel_log_obligatorio;
t_config* kernel_config;

int fd_kernel;
int fd_memoria;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_entradasalida;

char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* algoritmo_planificacion;
t_algoritmo ALGORITMO_PLANIFICACION;
float QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS_EN_CARACTERES;
int* INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;

// ------ Listas ------
t_list* lista_new;
t_list* lista_ready;
t_list* lista_execute;
t_list* lista_blocked;
t_list* lista_exit;
t_list* lista_ready_prioridad;

t_list* lista_instancias_global;

t_list* lista_instructions;
t_list* lista_recursos;

// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_exit;

pthread_mutex_t mutex_lista_ready_prioridad;

pthread_mutex_t mutex_process_id;
pthread_mutex_t mutex_pausa;
pthread_mutex_t mutex_verificador;

pthread_mutex_t mutex_lista_instancias_global;

pthread_mutex_t mutex_lista_instrucciones;
pthread_mutex_t mutex_lista_recursos;

int process_id;
int var_pausa;

bool flag_verificador;
int var_verificador;

// ------ SEMAFOROS ------
sem_t sem_pausa;
sem_t sem_enviar_interrupcion_consola;
sem_t sem_enviar_interrupcion_quantum;
sem_t sem_enviar_interrupcion_prioridad;
sem_t sem_estructura_iniciada;
sem_t sem_estructura_liberada;
sem_t sem_procesos_en_memoria;

void escuchar_ES();
int server_escucha_entrada_salida();
void enviar_peticion_nombre_y_tipo(int copia_socket); 


#endif /* KERNEL_H_ */



