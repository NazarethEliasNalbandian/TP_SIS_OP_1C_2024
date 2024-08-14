#ifndef K_GESTOR_H_
#define K_GESTOR_H_

#include "../../utils/include/shared.h"

typedef enum{
   FIFO,
   ROUNDROBIN,
   VIRTUALROUNDROBIN
}t_algoritmo;

typedef struct{
   char* recurso_name;
   int instancias;  // Instancias del recurso
   t_list* lista_bloqueados;   // Procesos a la espera de una instancia
   t_pcb* pcb_asignado;    // Procesos con instancias asignadas
   pthread_mutex_t mutex_bloqueados;
}t_recurso;

typedef enum {
	IO_GEN_SLEEP,
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ
} operacion_entradasalida;

typedef struct 
{	
	t_pcb* pcb;
	t_mochila* mochila;
	operacion_entradasalida operacion_solicitada;
}t_pcb_con_mochila; 


// Estructura para manejar múltiples instancias de entrada y salida
typedef struct {
    int socket;
    char* nombre;
	tipo_interfaz tipo;
	bool libre;
	t_queue* cola_de_espera; 
	t_pcb_con_mochila* pcb_con_interfaz_asignada;
	pthread_mutex_t mutex_espera;
} t_instancia_io;

typedef struct {
    int socket;
} t_instancia_args;

// VARIABLES GLOBALES
extern t_log* kernel_log_debug;
extern t_log* kernel_log_obligatorio;
extern t_config* kernel_config;


extern int fd_kernel;
extern int fd_memoria;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_entradasalida;

extern char* PUERTO_ESCUCHA;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern char* algoritmo_planificacion;
extern t_algoritmo ALGORITMO_PLANIFICACION;
extern float QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS_EN_CARACTERES;
extern int* INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION;

// ------ Listas ------
extern t_list* lista_new;
extern t_list* lista_ready;
extern t_list* lista_execute;
extern t_list* lista_blocked;
extern t_list* lista_exit;
extern t_list* lista_ready_prioridad;

extern t_list* lista_instancias_global;

extern t_list* lista_instructions;
extern t_list* lista_recursos;

// ------ PTHREAD_MUTEX ------
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_lista_exec;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_lista_exit;
extern pthread_mutex_t mutex_lista_ready_prioridad;

extern pthread_mutex_t mutex_process_id;
extern pthread_mutex_t mutex_pausa;
extern pthread_mutex_t mutex_verificador;

extern pthread_mutex_t mutex_lista_instancias_global;

extern pthread_mutex_t mutex_lista_instrucciones;
extern pthread_mutex_t mutex_lista_recursos;

//extern int identificador_PID;
extern int var_pausa;
extern sem_t sem_pausa;
extern pthread_mutex_t mutex_pausa;

extern int process_id;

extern bool flag_verificador;
extern int var_verificador;
//Para dar prioridad a la interrupcion por consola sobre la de quantum y en el hilo de interrupt si se finaliza proceso por DESALOJO PRO CONSOLA
// Para determinar si se recibio un proceso desalojado en cpu_dispatch

// ------ SEMAFOROS ------
extern sem_t sem_pausa;
extern sem_t sem_enviar_interrupcion_consola;
extern sem_t sem_enviar_interrupcion_quantum;
extern sem_t sem_enviar_interrupcion_prioridad;
extern sem_t sem_estructura_iniciada;
extern sem_t sem_estructura_liberada;
extern sem_t sem_procesos_en_memoria;


#endif /* K_GESTOR_H_ */



