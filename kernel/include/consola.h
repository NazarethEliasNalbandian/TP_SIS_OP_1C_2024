#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "k_gestor.h"

typedef enum{
    EJECUTAR_SCRIPT,
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO
}t_op_instruction;

typedef struct{
	char* instruction_name;
	t_op_instruction op_instruction;
	int instruction_n_param;
}t_instruction;

void _destruir_elementos(t_instruction* instruction);
void add_instruction_list(t_list* lista, char* instruccion, t_op_instruction op_instruction_code, int n_param);
void _inicializar_consola();
bool _validar_instruccion(char* leido);
void _f_iniciar_proceso(t_buffer* un_buffer);
void _pausar_planificadores();
void _finalizar_proceso_por_PID(char* un_pid);
void _iniciar_planificadores();
void _cambiar_grado_de_multiprogramacion(char* un_valor);
void imprimir_procesos_por_estado();
void _atender_instruccion_validada(char* leido);
void _leer_comandos();
void _finalizar_consola();
void _leer_consola();
void ejecutar_instrucciones_desde_archivo(char* path);

#endif /* CONSOLA_H_ */
