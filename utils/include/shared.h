#ifndef SHARED_H_
#define SHARED_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/process.h"
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/temporal.h"
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <readline/readline.h>
#include "commons/bitarray.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <readline/history.h>


typedef enum
{
   MENSAJE,
	PAQUETE,
	ERROR,
	INTERRUPT,

   ATENDER_RTA_KERNEL,

   HANDSHAKE,
   RTA_HANDSHAKE,
   CREAR_PROCESO_KM, // KERNEL A MEMORIA
   RPTA_CREAR_PROCESO_MK,


   	// KERNEL - CPU DISPATCH
	EJECUTAR_PROCESO_KC,
	ATENDER_INSTRUCCION_CPU,
	ATENDER_DESALOJO_PROCESO_CPU,
	ATENDER_INTERRUPT_PROCESO_CPU,
	ATENDER_EXIT,
	ATENDER_OUT_OF_MEMORY,
	ATENDER_IO_GEN_SLEEP,
	ATENDER_IO_STDIN_READ,
	ATENDER_IO_STDOUT_WRITE,
	ATENDER_IO_FS_CREATE,
	ATENDER_IO_FS_DELETE,
	ATENDER_IO_FS_TRUNCATE,
	ATENDER_IO_FS_WRITE,

	// KERNEL - CPU INTERRUPT
	FORZAR_DESALOJO_CPU_KERNEL,



   // CPU - MEMORIA
	PETICION_INFO_CPU_MEMORIA,
	PETICION_DE_INSTRUCCIONES_CPU_MEMORIA,
	CONSULTA_DE_PAGINA_CPU_MEMORIA,
	LECTURA_BLOQUE_CPU_MEMORIA,
	ESCRITURA_BLOQUE_CPU_MEMORIA,
	RESIZE_CPU_MEMORIA,


   //------- KERNEL - MEMORIA
   ESTRUCTURA_INICIADA_KERNEL_MEMORIA,
   ESTRUCTURA_LIBERADA_KERNEL_MEMORIA,

   	// KERNEL - ENTRADASALIDA
	NOMBRE_ENTRADA_SALIDA,
	FIN_ENTRADASALIDA,
	PCB_SLEEP,
	PCB_STDIN,
	PCB_STDOUT,
	PCB_FS_CREATE,
	PCB_FS_DELETE,
	PCB_FS_TRUNCATE,
	PCB_FS_WRITE,
	PCB_FS_READ,

   // ENTRADASALIA - MEMORIA
   LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA,
   ESCRITURA_BLOQUE_ENTRADASALIDA_MEMORIA,
   PETICION_INFO_ENTRADASALIDA_MEMORIA


}op_code;

typedef enum {

	T_INT,
	T_STRING,
	T_SIZE_T,
	T_UINT32,
	T_UINT8,
	T_CHAR
}tipo_dato_parametro;

typedef struct {
	char* instruccionAsociada;
	int cantidad_parametros_inicial;
	t_queue* parametros;
} t_mochila;

typedef enum {
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS

} tipo_interfaz;

typedef enum{
   NEW,//=0
   READY,//=1
   EXEC,//=2
   BLOCKED,//=3
   EXIT,//=4
   READY_PRIORIDAD
}est_pcb;

typedef struct{
	uint32_t PC;
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
	uint32_t SI;
	uint32_t DI;
}t_registrosCPU;

typedef enum{
   SUCCESS,  // 0
   INVALID_RESOURCE,  // 1
   INVALID_INTERFACE, // 2
   OUT_OF_MEMORY, // 3
   INTERRUPTED_BY_USER // 4
}t_motivo_exit;

typedef struct{
    int pID;
    int verificador;
    t_registrosCPU* r_cpu;
}t_contexto;


typedef struct {
    char* pseudo_c;
    char* fst_param;
    char* snd_param;
    char* third_param;
    char* fourth_param;
    char* fifth_param;
} t_instruccion_codigo;

typedef struct{
   int pid;
   float quantum;
   t_registrosCPU* registros_CPU;
   int verificador;
   char* path;
   est_pcb estado;
   t_list* lista_recursos_pcb;
   t_motivo_exit motivo_exit;
   t_temporal* timer;   
   bool flag_proceso_venia_de_ready_prioridad;
   bool flag_proceso_desalojado_por_prioridad;
   bool flag_proceso_desalojado;
   bool flag_finalizar_proceso;
   bool flag_cancelar_quantum;
}t_pcb;
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

// SOCKETS 
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char * puerto, t_log* un_log, char * msj_server);
int esperar_cliente(int socket_servidor, t_log* un_log, char* msj);
int recibir_operacion(int socket_cliente);
void liberar_conexion(int socket_cliente);


// SERIALIZACION
t_paquete* crear_super_paquete(op_code code_op);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete* un_paquete);
void crear_buffer(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);

// CARGAS
void cargar_int_al_super_paquete(t_paquete* paquete, int numero);
void cargar_string_al_super_paquete(t_paquete* paquete, char* string);
void cargar_choclo_al_super_paquete(t_paquete* paquete, void* choclo, int size);
void cargar_uint8_al_super_paquete(t_paquete* un_paquete, uint8_t uint8_value);
void cargar_uint32_al_super_paquete(t_paquete* un_paquete, uint32_t uint32_value);
void cargar_size_t_al_super_paquete(t_paquete* un_paquete, size_t size_t_value);
void cargar_char_al_super_paquete(t_paquete* paquete, char caracter);

void agregar_registros_a_paquete(t_paquete * un_paquete, t_registrosCPU* registroRecibido);

// EXTRACCIÓN
t_buffer* recibir_paquete(int conexion);
void* recibir_buffer(int* size, int socket_cliente);
int recibir_int_del_buffer(t_buffer* coso);
char* recibir_string_del_buffer(t_buffer* coso);
void* recibir_choclo_del_buffer(t_buffer* coso);
void* recibir_generico_del_buffer(t_buffer* un_buffer);
uint8_t recibir_uint8_del_buffer(t_buffer* un_buffer);
uint32_t recibir_uint32_del_buffer(t_buffer* un_buffer);
size_t recibir_size_t_del_buffer(t_buffer* un_buffer);
char recibir_char_del_buffer(t_buffer* un_buffer);
void recibir_mensaje(int,t_log*);

void recibir_mochila(t_buffer * unBuffer, t_mochila* mochilaRecibida, t_log* logger);
void recibir_contexto(t_buffer * unBuffer, t_contexto* contextoRecibido);
void recibir_registros(t_buffer* unBuffer, t_contexto* contextoRecibido);

// SERVICIOS
void limpiar_buffer_entrada();
void safe_free(void* elemento);
void ejecutar_en_un_hilo_nuevo_detach(void (*f)(void*) ,void* struct_arg);
void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg);
int convertirInterfazAEnum(const char* tipo_entradasalida);
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo);

// DESTRUCCIÓN
void destruir_contexto_por_param(t_contexto* contexto);
void destruir_buffer(t_buffer* buffer);

#endif
