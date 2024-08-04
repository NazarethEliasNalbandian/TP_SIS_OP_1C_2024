#ifndef MEMORIA_ENTRADASALIDA_H_
#define MEMORIA_ENTRADASALIDA_H_

#include "m_gestor.h"

void atender_memoria_entradasalida(void * instancia);
void _escribir_valor_en_dir_fisica(t_buffer* un_buffer, int socket);
void _leer_valor_de_dir_fisica(t_buffer* un_buffer, int socket);
void enviar_a_entradasalida_respuesta_por_pedido_de_escritura_en_memoria(int socket);
void _enviar_uint32_por_lectura(uint32_t valor, tipo_dato_parametro tipo_dato, int socket);
void _enviar_uint8_por_lectura(uint8_t valor, tipo_dato_parametro tipo_dato, int socket);
void _enviar_string_por_lectura(char* valor, int socket);
void _enviar_valor_por_lectura(void* valor, int socket, size_t tamanio);

#endif /* MEMORIA_ENTRADASALIDA_H_ */
