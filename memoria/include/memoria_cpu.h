#ifndef MEMORIA_CPU_H_
#define MEMORIA_CPU_H_

#include "m_gestor.h"

void atender_memoria_cpu();
void atender_peticion_de_instruccion(t_buffer* un_buffer);
void atender_consulta_de_pagina(t_buffer* unBuffer);
void leer_valor_de_dir_fisica(t_buffer* un_buffer);
void escribir_valor_en_dir_fisica(t_buffer* un_buffer);
void atender_resize(t_buffer* un_buffer);

void enviar_instruccion(char* instruccion);
void enviar_respuesta_por_consulta_de_pagina(int respuesta_a_cpu);
void enviar_uint32_por_lectura(uint32_t valor, tipo_dato_parametro tipo_dato);
void enviar_uint8_por_lectura(uint8_t valor, tipo_dato_parametro tipo_dato);
void enviar_valor_por_lectura(void* valor, tipo_dato_parametro tipo_dato, size_t tamanio);
void enviar_string_por_lectura(char* valor, tipo_dato_parametro tipo_dato);
void enviar_respuesta_por_escritura();
void enviar_respuesta_por_resize();
void enviar_OUT_OF_MEMORY();

#endif /* MEMORIA_CPU_H_ */
