#ifndef SERVICIOS_CPU_H_
#define SERVICIOS_CPU_H_

#include "cpu_gestor.h"

nombre_instruccion_comando convertirStringAEnum(char* nombre_instruccion);
bool validador_de_header(char* header_string);
void enviar_contexto_a_kernel_con_desalojo_cpu();
void agregar_contexto_a_paquete(t_paquete * un_paquete);
bool hay_io();
bool es_registro_uint32(char* registro);
bool es_registro_uint8(char* registro);
void asignarRegistroExtended(char* registro, uint32_t** valor_a_guardar);
void asignarRegistro(char* registro, uint8_t** valor_a_guardar);
int obtenerIntDeRegistro(char* registro, uint32_t** er, uint8_t**r);
size_t obtenerSizeTDeRegistro(char* registro, uint32_t** er, uint8_t**r);
void escribir_registro(char* registro_a_escribir,int valor, uint32_t* registroAEscribirUint32, uint8_t* registroAEscribirUint8);
u_int32_t obtenerUint32DeRegistro(char* registro, uint32_t** er);
u_int8_t obtenerUint8DeRegistro(char* registro, uint8_t**r);
void liberar_instruccion_split(char** instruccion_split);

#endif