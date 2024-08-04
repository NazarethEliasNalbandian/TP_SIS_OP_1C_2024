#ifndef CPU_H_
#define CPU_H_

#include "cpu_gestor.h"
#include "incializar_cpu.h"
#include "cpu_kernel_dispatch.h"
#include "cpu_kernel_interrupt.h"
#include "cpu_memoria.h"
#include "finalizar_cpu.h"
#include "dic_operaciones.h"


t_log* cpu_log_debug;
t_log* cpu_log_obligatorio;;
t_config* cpu_config;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANT_ENTRADAS_TLB;
char* ALGORITMO_TLB;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;

sem_t sem_fetch;
sem_t sem_decode;
sem_t sem_execute;
sem_t sem_resize;
sem_t sem_val_leido;
sem_t sem_val_escrito;
sem_t sem_sol_marco;
sem_t sem_rta_kernel;
sem_t sem_inst;

pthread_mutex_t mutex_interruptFlag;
pthread_mutex_t mutex_manejo_contexto;
pthread_mutex_t mutex_ejecute_wait_y_no_hay_instancias;

int TAM_PAGINA;
int desplazamiento;

int interrupt_pid;
int interrupt_verificador;
char* interrupt_motivo;
bool interrupt_flag;
bool desalojar;
bool fallo_algoritmo;
char* instruccion;
char* rta_m_resize;
char* rta_kernel;
bool flag_exit;

char* valor_leido_string;
uint32_t valor_leido_uint32;
uint8_t valor_leido_uint8;

char* valor_escrito;

char** op_autorizada;
char** instruccion_split;

t_list* TLB;
t_list* TLB_indices;
int nro_marco;

t_contexto* un_contexto;
op_code tipo_desalojo;
nombre_instruccion_comando nombre_instruccion_enum;

bool ejecute_wait_y_no_hay_instancias;


pthread_mutex_t mutex_interruptFlag;
pthread_mutex_t mutex_manejo_contexto;
pthread_mutex_t mutex_ejecute_wait_y_no_hay_instancias;
pthread_mutex_t mutex_interrupt_pid;
pthread_mutex_t mutex_interrupt_verificador;
pthread_mutex_t mutex_interrupt_motivo;
pthread_mutex_t mutex_desalojar;
pthread_mutex_t mutex_fallo_algoritmo;
pthread_mutex_t mutex_instruccion;
pthread_mutex_t mutex_rta_m_resize;
pthread_mutex_t mutex_rta_kernel;
pthread_mutex_t mutex_flag_exit;
pthread_mutex_t mutex_valor_leido_string;
pthread_mutex_t mutex_valor_leido_uint32;
pthread_mutex_t mutex_valor_leido_uint8;
pthread_mutex_t mutex_valor_escrito;
pthread_mutex_t mutex_op_autorizada;
pthread_mutex_t mutex_instruccion_split;
pthread_mutex_t mutex_TLB;
pthread_mutex_t mutex_TLB_indices;
pthread_mutex_t mutex_nro_marco;
pthread_mutex_t mutex_tipo_desalojo;
pthread_mutex_t mutex_nombre_instruccion_enum;

bool hay_io_gen;
sem_t sem_nro_marco;

#endif
