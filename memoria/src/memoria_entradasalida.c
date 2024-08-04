#include "../include/memoria_entradasalida.h"
#include "../include/espacio_usuario.h"

void atender_memoria_entradasalida(void * instancia){
	t_instancia_io* nueva_instancia = (t_instancia_io *) instancia;
	bool control_key = 1;

	while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(nueva_instancia->socket);
		switch (cod_op) {
		case LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA: //[int pid][int dir_fisica][size_t tamanio]
			unBuffer = __recibiendo_super_paquete(nueva_instancia->socket);
			_leer_valor_de_dir_fisica(unBuffer, nueva_instancia->socket); 
			break;
		case ESCRITURA_BLOQUE_ENTRADASALIDA_MEMORIA: ////[int pid][int dir_fisica][size_t tamanio][void* info]
			unBuffer = __recibiendo_super_paquete(nueva_instancia->socket);
			_escribir_valor_en_dir_fisica(unBuffer, nueva_instancia->socket);
			break;
		case -1:
			log_error(memoria_log_debug, "DESCONEXION DE ENTRADASALIDA");
			control_key = 0;
			break;
		default:
			log_warning(memoria_log_debug,"OPERACION DESCONOCIDA DE ENTRADASALIDA CON SOCKET %d", nueva_instancia->socket);
			break;
		}
	}
}

void _escribir_valor_en_dir_fisica(t_buffer* un_buffer, int socket){

	int pid = __recibir_int_del_buffer(un_buffer);
	int dir_fisica = __recibir_int_del_buffer(un_buffer);
	size_t tamanio = extraer_size_t_del_buffer(un_buffer);
	tipo_dato_parametro tipo_dato = __recibir_int_del_buffer(un_buffer);
	void* valor = __recibir_choclo_del_buffer(un_buffer);

	destruir_buffer(un_buffer);

	escribir_data_en_dir_fisica(pid, dir_fisica, valor, tamanio);

	//Enviar valor a CPU
	enviar_a_entradasalida_respuesta_por_pedido_de_escritura_en_memoria(socket);
	if(valor != NULL){

		free(valor);
		valor = NULL;
	}

}

void _leer_valor_de_dir_fisica(t_buffer* un_buffer, int socket){

	int pid = __recibir_int_del_buffer(un_buffer);
	int dir_fisica = __recibir_int_del_buffer(un_buffer);
	size_t tamanio = extraer_size_t_del_buffer(un_buffer);
	tipo_dato_parametro tipo_dato = __recibir_int_del_buffer(un_buffer);

	if(tipo_dato == T_UINT32){
		uint32_t valor_uint32 = leer_uint32_de_dir_fisica(pid, dir_fisica);
		_enviar_uint32_por_lectura(valor_uint32, tipo_dato, socket);
	}else if(tipo_dato == T_UINT8){
		uint8_t valor_uint8 = leer_uint8_de_dir_fisica(pid, dir_fisica);
		_enviar_uint8_por_lectura(valor_uint8, tipo_dato, socket);
	}else if(tipo_dato == T_STRING){
		void* valor = leer_data_de_dir_fisica(pid, dir_fisica, tamanio);
		_enviar_valor_por_lectura(valor, socket, tamanio);
	}
	destruir_buffer(un_buffer);
}

// ENVIOS

void enviar_a_entradasalida_respuesta_por_pedido_de_escritura_en_memoria(int socket){
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(ESCRITURA_BLOQUE_ENTRADASALIDA_MEMORIA);
	__cargar_string_al_super_paquete(un_paquete, "SE ESCRIBIO EN LA MEMORIA");
	enviar_paquete(un_paquete, socket);
	eliminar_paquete(un_paquete);
}

void _enviar_uint32_por_lectura(uint32_t valor, tipo_dato_parametro tipo_dato, int socket){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
    __cargar_uint32_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, socket);
    eliminar_paquete(un_paquete);
}

void _enviar_uint8_por_lectura(uint8_t valor, tipo_dato_parametro tipo_dato, int socket){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
    __cargar_uint8_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, socket);
    eliminar_paquete(un_paquete);
}

void _enviar_string_por_lectura(char* valor, int socket){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA);
    __cargar_string_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, socket);
    eliminar_paquete(un_paquete);

	// if(valor != NULL){
	// 	free(valor);
	// 	valor = NULL;
	// }
}

void _enviar_valor_por_lectura(void* valor, int socket, size_t tamanio){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA);
    __cargar_choclo_al_super_paquete(un_paquete, valor, tamanio);
    enviar_paquete(un_paquete, socket);
    eliminar_paquete(un_paquete);

}