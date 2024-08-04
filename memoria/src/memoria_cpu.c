#include "../include/memoria_cpu.h"
#include "../include/proceso.h"

void atender_memoria_cpu(){

	bool control_key = 1;

	//Enviar tamaño de pagina a CPU _
	t_paquete* un_paquete = __crear_super_paquete(PETICION_INFO_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, TAM_PAGINA);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
	while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		case PETICION_DE_INSTRUCCIONES_CPU_MEMORIA: //[int pid][int ip]
			unBuffer = __recibiendo_super_paquete(fd_cpu);
			atender_peticion_de_instruccion(unBuffer);
			break;
		case CONSULTA_DE_PAGINA_CPU_MEMORIA: //[int pid][int nro_pagina]
			unBuffer = __recibiendo_super_paquete(fd_cpu);
			atender_consulta_de_pagina(unBuffer); 
			break;
		case LECTURA_BLOQUE_CPU_MEMORIA: //[int pid][int dir_fisica]
			unBuffer = __recibiendo_super_paquete(fd_cpu);
			leer_valor_de_dir_fisica(unBuffer); 
			break;
		case ESCRITURA_BLOQUE_CPU_MEMORIA: ////[int pid][int dir_fisica][uint32_t info]
			unBuffer = __recibiendo_super_paquete(fd_cpu);
			escribir_valor_en_dir_fisica(unBuffer);
			break;
		case RESIZE_CPU_MEMORIA:
			unBuffer = __recibiendo_super_paquete(fd_cpu); //[int pid][int nuevo_tamaño_en_bytes]
			atender_resize(unBuffer);
			break;
		case -1:
			log_error(memoria_log_debug, "DESCONEXION DE CPU");
			control_key = 0;
			break;
		default:
			log_warning(memoria_log_debug,"OPERACION DESCONOCIDA DE CPU");
			break;
		}
	}
}

void atender_peticion_de_instruccion(t_buffer* un_buffer){
	int pid = __recibir_int_del_buffer(un_buffer);
	uint32_t ip = extraer_uint32_del_buffer(un_buffer);
	destruir_buffer(un_buffer);

	//Buscar proceso por el PID
	t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(un_proceso, ip);

	log_info(memoria_log_debug, "<PID:%d> <IP:%d> <%s>", pid, ip, instruccion);

	//Enviar_instruccion a CPU
	enviar_instruccion(instruccion);
}

void atender_consulta_de_pagina(t_buffer* unBuffer){
	int pid = __recibir_int_del_buffer(unBuffer);
	int nro_pagina = __recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	t_pagina* una_pagina = pag_obtener_pagina_completa(un_proceso, nro_pagina);

	printf("DATOS DE LA PAGINA: \n");
	printf("NRO MARCO: %d", una_pagina->nro_marco);
	printf("NRO PAGINA: %d\n", una_pagina->nro_pagina);

	int respuesta_a_cpu;
	if(una_pagina != NULL){
		respuesta_a_cpu = una_pagina->nro_marco;
		t_marco* un_marco = obtener_marco_por_nro_marco(una_pagina->nro_marco);
		logg_acceso_a_tabla_de_paginas(pid, nro_pagina, un_marco->nro_marco);
		log_info(memoria_log_debug, "PAGINA ENCONTRADA <PID:%d> <Pag:%d> <Marco:%d>", pid, nro_pagina, una_pagina->nro_marco);
	}
	else{
		respuesta_a_cpu = -1;
		log_warning(memoria_log_debug, "PÁGINA NO ENCONTRADA <PID:%d> <Pag:%d>", pid, nro_pagina);
	}
	enviar_respuesta_por_consulta_de_pagina(respuesta_a_cpu);
}

void leer_valor_de_dir_fisica(t_buffer* un_buffer){
	int pid = __recibir_int_del_buffer(un_buffer);
	int dir_fisica = __recibir_int_del_buffer(un_buffer);
	size_t tamanio = extraer_size_t_del_buffer(un_buffer);
	tipo_dato_parametro tipo_dato = __recibir_int_del_buffer(un_buffer);
	destruir_buffer(un_buffer);

	if(tipo_dato == T_UINT32){
		uint32_t valor_uint32 = leer_uint32_de_dir_fisica(pid, dir_fisica);
		enviar_uint32_por_lectura(valor_uint32, tipo_dato);
	}else if(tipo_dato == T_UINT8){
		uint8_t valor_uint8 = leer_uint8_de_dir_fisica(pid, dir_fisica);
		enviar_uint8_por_lectura(valor_uint8, tipo_dato);
	}else if(tipo_dato == T_STRING){
		void* valor = leer_data_de_dir_fisica(pid, dir_fisica, tamanio);
		enviar_valor_por_lectura(valor, tipo_dato, tamanio);
	}
	
}

void escribir_valor_en_dir_fisica(t_buffer* un_buffer){
	int pid = __recibir_int_del_buffer(un_buffer);
	int dir_fisica = __recibir_int_del_buffer(un_buffer);
	size_t tamanio = extraer_size_t_del_buffer(un_buffer);
	tipo_dato_parametro tipo_dato = __recibir_int_del_buffer(un_buffer);
	void* valor = __recibir_choclo_del_buffer(un_buffer);

	log_trace(memoria_log_debug,"DIR: %d", dir_fisica);

	destruir_buffer(un_buffer);

	if(tipo_dato == T_UINT32){
		uint32_t* valor_uint_32;
		valor_uint_32 = (uint32_t*) valor;
		//Escribir en espacio de usuario
		escribir_data_en_dir_fisica(pid, dir_fisica, valor, tamanio);
	} else if (tipo_dato == T_UINT8){
		uint8_t* valor_uint_8;
		valor_uint_8 = (uint8_t*) valor;

		//Escribir en espacio de usuario
		escribir_data_en_dir_fisica(pid, dir_fisica, valor, tamanio);

	} else if (tipo_dato == T_STRING)
	{
		char* valor_string;
		valor_string = (char*) valor;

		//Escribir en espacio de usuario
		escribir_data_en_dir_fisica(pid, dir_fisica, valor, tamanio);

	}

	//Enviar valor a CPU
	enviar_respuesta_por_escritura();

	if(valor != NULL){

		free(valor);
		valor = NULL;
	}
}

void atender_resize(t_buffer* un_buffer) {
	int pid = __recibir_int_del_buffer(un_buffer);
	int nuevo_tamanio = __recibir_int_del_buffer(un_buffer);

	destruir_buffer(un_buffer);

	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	int cantidad_de_paginas_actuales = cantidad_paginas_de_un_proceso(un_proceso);

	int nueva_cantidad_de_paginas = (int) ceil(((double) nuevo_tamanio)/ ((double) TAM_PAGINA));
	int cantidad_de_paginas_a_agregar = nueva_cantidad_de_paginas - cantidad_de_paginas_actuales;
	int cantidad_de_paginas_a_eliminar = cantidad_de_paginas_a_agregar * (-1);
	int resultado;

	if(cantidad_de_paginas_a_agregar >= 0){
		resultado = agregar_paginas_a_un_proceso(un_proceso, cantidad_de_paginas_a_agregar);
		if(resultado == -1)
			return;
		logg_ampliacion_proceso(pid, cantidad_paginas_de_un_proceso(un_proceso) * TAM_PAGINA, cantidad_de_paginas_a_agregar * TAM_PAGINA);
	}
	else{
		eliminar_paginas_a_un_proceso(un_proceso, cantidad_de_paginas_a_eliminar); 
		logg_reduccion_proceso(pid, cantidad_paginas_de_un_proceso(un_proceso) * TAM_PAGINA, cantidad_de_paginas_a_eliminar * TAM_PAGINA);
	}

	enviar_respuesta_por_resize();
	return;
}

//============ENVIOS A CPU=======================

void enviar_instruccion(char* instruccion){
	// M -> CPU : [char* ]
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(PETICION_DE_INSTRUCCIONES_CPU_MEMORIA);
	__cargar_string_al_super_paquete(un_paquete, instruccion);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_respuesta_por_consulta_de_pagina(int respuesta_a_cpu){
	// M -> CPU : [int nro_bloque_o_(-1)_pagefault]
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(CONSULTA_DE_PAGINA_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, respuesta_a_cpu);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_uint32_por_lectura(uint32_t valor, tipo_dato_parametro tipo_dato){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
    __cargar_uint32_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
}

void enviar_uint8_por_lectura(uint8_t valor, tipo_dato_parametro tipo_dato){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
    __cargar_uint8_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);
}

void enviar_string_por_lectura(char* valor, tipo_dato_parametro tipo_dato){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
    __cargar_string_al_super_paquete(un_paquete, valor);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);

	if(valor != NULL){

		free(valor);
		valor = NULL;
	}
}


void enviar_valor_por_lectura(void* valor, tipo_dato_parametro tipo_dato, size_t tamanio){
    // M -> CPU : [void* valor]
    retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
	__cargar_int_al_super_paquete(un_paquete, tipo_dato);
	__cargar_choclo_al_super_paquete(un_paquete, valor, tamanio);
    enviar_paquete(un_paquete, fd_cpu);
    eliminar_paquete(un_paquete);

	if(valor != NULL){

		free(valor);
		valor = NULL;
	}
}

void enviar_respuesta_por_escritura(){
	//M -> CPU : [char* OK]
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(ESCRITURA_BLOQUE_CPU_MEMORIA);
	__cargar_string_al_super_paquete(un_paquete, "SE ESCRIBIO EN LA MEMORIA");
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_respuesta_por_resize(){
	//M -> CPU : [char* OK]
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(RESIZE_CPU_MEMORIA);
	__cargar_string_al_super_paquete(un_paquete, "OK");
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_OUT_OF_MEMORY(){
	retardo_respuesta();
	t_paquete* un_paquete = __crear_super_paquete(RESIZE_CPU_MEMORIA);
	__cargar_string_al_super_paquete(un_paquete, "OUT OF MEMORY");
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}
