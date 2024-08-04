#ifndef C_GESTOR_H_
#define C_GESTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/list.h"
#include "commons/string.h"
#include "commons/process.h"

#include "../../utils/include/shared.h"

typedef struct {
    int pid;
    int nro_pagina;
    int nro_marco;
}elemento_TLB;

typedef enum {
    SET,
    SUM,
    SUB,
    JNZ,
    IO_GEN_SLEEP,
    RESIZE,
    WAIT,
    SIGNAL,
    MOV_IN,
    MOV_OUT,
    COPY_STRING,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    EXITT
} nombre_instruccion_comando;

extern t_log* cpu_log_debug;
extern t_log* cpu_log_obligatorio;
extern t_config* cpu_config;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANT_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

extern sem_t sem_fetch;
extern sem_t sem_decode;
extern sem_t sem_execute;
extern sem_t sem_resize;
extern sem_t sem_val_leido;
extern sem_t sem_val_escrito;
extern sem_t sem_sol_marco;
extern sem_t sem_rta_kernel;
extern sem_t sem_inst;
extern sem_t sem_nro_marco;

extern pthread_mutex_t mutex_interruptFlag;
extern pthread_mutex_t mutex_manejo_contexto; 

extern int TAM_PAGINA;
extern int desplazamiento;

extern bool interrupt_flag;
extern int interrupt_pid;
extern int interrupt_verificador;
extern char* interrupt_motivo;
extern bool desalojar;
extern bool fallo_algoritmo;
extern char* instruccion;
extern char* rta_m_resize;
extern char* rta_kernel;
extern bool flag_exit;

extern char* valor_leido_string;
extern uint32_t valor_leido_uint32;
extern uint8_t valor_leido_uint8;

extern char* valor_escrito;

extern char** op_autorizada;
extern char** instruccion_split;

extern t_list* TLB;
extern t_list* TLB_indices;
extern int nro_marco;

extern t_contexto* un_contexto;
extern op_code tipo_desalojo;
extern nombre_instruccion_comando nombre_instruccion_enum;

extern bool ejecute_wait_y_no_hay_instancias;
extern pthread_mutex_t mutex_ejecute_wait_y_no_hay_instancias;


extern pthread_mutex_t mutex_interruptFlag;
extern pthread_mutex_t mutex_manejo_contexto;
extern pthread_mutex_t mutex_ejecute_wait_y_no_hay_instancias;
extern pthread_mutex_t mutex_interrupt_pid;
extern pthread_mutex_t mutex_interrupt_verificador;
extern pthread_mutex_t mutex_interrupt_motivo;
extern pthread_mutex_t mutex_desalojar;
extern pthread_mutex_t mutex_fallo_algoritmo;
extern pthread_mutex_t mutex_instruccion;
extern pthread_mutex_t mutex_rta_m_resize;
extern pthread_mutex_t mutex_rta_kernel;
extern pthread_mutex_t mutex_flag_exit;
extern pthread_mutex_t mutex_valor_leido_string;
extern pthread_mutex_t mutex_valor_leido_uint32;
extern pthread_mutex_t mutex_valor_leido_uint8;
extern pthread_mutex_t mutex_valor_escrito;
extern pthread_mutex_t mutex_op_autorizada;
extern pthread_mutex_t mutex_instruccion_split;
extern pthread_mutex_t mutex_TLB;
extern pthread_mutex_t mutex_TLB_indices;
extern pthread_mutex_t mutex_nro_marco;
extern pthread_mutex_t mutex_tipo_desalojo;
extern pthread_mutex_t mutex_nombre_instruccion_enum;

extern bool hay_io_gen;

#endif 