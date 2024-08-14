#include "../include/shared.h"


// SOCKET

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = 0;

	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("Error al conectar el socket ");

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* puerto, t_log* un_log, char* msj_server)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);

	if( setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	 	printf("setsockopt(SO_REUSEADDR) failed");

	bind(socket_servidor,servinfo->ai_addr, servinfo->ai_addrlen );

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(un_log, "SERVER: %s", msj_server);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* un_log, char* msj)
{
	int socket_cliente;

	socket_cliente = accept(socket_servidor, NULL, NULL);

	log_info(un_log, "SE CONECTO EL CLIENTE: %s!", msj);

	return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
	{
		return cod_op;
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

// PROTOCOLO

t_paquete* crear_paquete(op_code code_op){
	t_paquete* super_paquete = malloc(sizeof(t_paquete));
	super_paquete->codigo_operacion = code_op;
	crear_buffer(super_paquete);
	return  super_paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


// EXTRACCIÓN


t_buffer* recibir_paquete(int conexion){
	t_buffer* unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream = recibir_buffer(&size, conexion);
	unBuffer->size = size;
	return unBuffer;
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void* recibir_generico_del_buffer(t_buffer* un_buffer){
	if(un_buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(un_buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_generico;
	void* generico;
	memcpy(&size_generico, un_buffer->stream, sizeof(int));
	generico = malloc(size_generico);
	memcpy(generico, un_buffer->stream + sizeof(int), size_generico);

	int nuevo_size = un_buffer->size - sizeof(int) - size_generico;
	if(nuevo_size == 0){
		un_buffer->size = 0;
		free(un_buffer->stream);
		un_buffer->stream = NULL;
		return generico;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_CHICLO]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		exit(EXIT_FAILURE);
	}
	void* nuevo_generico = malloc(nuevo_size);
	memcpy(nuevo_generico, un_buffer->stream + sizeof(int) + size_generico, nuevo_size);
	free(un_buffer->stream);
	un_buffer->stream = nuevo_generico;
	un_buffer->size = nuevo_size;
	// free(nuevo_generico);

	return generico;
}

int recibir_int_del_buffer(t_buffer* coso){
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int));

	int nuevo_size = coso->size - sizeof(int);
	if(nuevo_size == 0){

		if(coso->stream != NULL){
			free(coso->stream);
			coso->stream = NULL;
		}

		coso->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_INT]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int), nuevo_size);
	if(coso->stream != NULL){
		free(coso->stream);
		coso->stream = NULL;
	}
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}

char* recibir_string_del_buffer(t_buffer* coso){

    //----------------- Formato Inicial----------------------------
	if(coso->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(coso->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_string;
	char* string;
	memcpy(&size_string, coso->stream, sizeof(int));
	//string = malloc(sizeof(size_string));
	string = malloc(size_string);
	memcpy(string, coso->stream + sizeof(int), size_string);

	int nuevo_size = coso->size - sizeof(int) - size_string;
	if(nuevo_size == 0){
		if(coso->stream != NULL){
			free(coso->stream);
			coso->stream = NULL;
		}
		coso->stream = NULL;
		coso->size = 0;
		return string;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_STRING]: BUFFER CON TAMAÑO NEGATIVO\n\n");

		if(string != NULL){
			free(string);
			string = NULL;
		}
		//return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
		exit(EXIT_FAILURE);
	}
	void* nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int) + size_string, nuevo_size);
	if(coso->stream != NULL){
		free(coso->stream);
		coso->stream = NULL;
	}
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return string;
}

size_t recibir_size_t_del_buffer(t_buffer* un_buffer){
	size_t* un_size_t = recibir_generico_del_buffer(un_buffer);
	size_t valor_retorno = *un_size_t;
	if(un_size_t != NULL){

		free(un_size_t);
		un_size_t = NULL;
	}
	return valor_retorno;
}

uint8_t recibir_uint8_del_buffer(t_buffer* un_buffer){
	uint8_t* un_entero_8 = recibir_generico_del_buffer(un_buffer);
	uint8_t valor_retorno = *un_entero_8;

	if(un_entero_8 != NULL){
		free(un_entero_8);
		un_entero_8 = NULL;
	}
	return valor_retorno;
}

uint32_t recibir_uint32_del_buffer(t_buffer* un_buffer){
	uint32_t* un_entero_32 = recibir_generico_del_buffer(un_buffer);
	uint32_t valor_retorno = *un_entero_32;

	if(un_entero_32 != NULL){
		free(un_entero_32);
		un_entero_32 = NULL;
	}

	return valor_retorno;
}

char recibir_char_del_buffer(t_buffer* un_buffer){
	char* un_char = recibir_generico_del_buffer(un_buffer);
	char valor_retorno = *un_char;

	if(un_char != NULL){
		free(un_char);
		un_char = NULL;
	}
	return valor_retorno;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

void recibir_contexto(t_buffer * unBuffer, t_contexto* contextoRecibido) {
	contextoRecibido->pID = recibir_int_del_buffer(unBuffer);
	contextoRecibido->verificador = recibir_int_del_buffer(unBuffer);
	recibir_registros(unBuffer, contextoRecibido);
}

void recibir_mochila(t_buffer *unBuffer, t_mochila* mochilaRecibida, t_log* logger) {
    mochilaRecibida->instruccionAsociada = recibir_string_del_buffer(unBuffer);
    mochilaRecibida->cantidad_parametros_inicial = recibir_int_del_buffer(unBuffer);

    tipo_dato_parametro TIPO_DATO;
    int i;
    for (i = 0; i < mochilaRecibida->cantidad_parametros_inicial; i++) {
        TIPO_DATO = recibir_int_del_buffer(unBuffer);

        switch (TIPO_DATO) {
            case T_INT: {
                int* valor_int = malloc(sizeof(int));
                if (valor_int == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro INT");
                    continue; // Continuar con el siguiente parámetro
                }
                *valor_int = recibir_int_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_int);
                log_info(logger, "CARGUE VALOR INT: %d", *valor_int);
                break;
            }
            case T_STRING: {
                char* valor_string = recibir_string_del_buffer(unBuffer);
                if (valor_string == NULL) {
                    log_error(logger, "No se pudo recibir el parámetro STRING");
                    continue; // Continuar con el siguiente parámetro
                }
                queue_push(mochilaRecibida->parametros, valor_string);
                log_info(logger, "CARGUE VALOR STRING: %s", valor_string);
                break;
            }
            case T_SIZE_T: {
                size_t* valor_size_t = malloc(sizeof(size_t));
                if (valor_size_t == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro SIZE_T");
                    continue; // Continuar con el siguiente parámetro
                }
                *valor_size_t = recibir_size_t_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_size_t);
                log_info(logger, "CARGUE VALOR SIZE_T: %zu", *valor_size_t);
                break;
            }
            case T_UINT32: {
                uint32_t* valor_uint32 = malloc(sizeof(uint32_t));
                if (valor_uint32 == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro UINT32");
                    continue; // Continuar con el siguiente parámetro
                }
                *valor_uint32 = recibir_uint32_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_uint32);
                log_info(logger, "CARGUE VALOR UINT32: %u", *valor_uint32);
                break;
            }
            case T_UINT8: {
                uint8_t* valor_uint8 = malloc(sizeof(uint8_t));
                if (valor_uint8 == NULL) {
                    log_error(logger, "No se pudo asignar memoria para el parámetro UINT8");
                    continue; // Continuar con el siguiente parámetro
                }
                *valor_uint8 = recibir_uint8_del_buffer(unBuffer);
                queue_push(mochilaRecibida->parametros, valor_uint8);
                log_info(logger, "CARGUE VALOR UINT8: %u", *valor_uint8);
                break;
            }
            default:
                log_error(logger, "TIPO DATO NO VALIDO");
                break;
        }
    }
	
}

void recibir_registros(t_buffer* unBuffer, t_contexto* contextoRecibido){
	contextoRecibido->r_cpu->PC = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->AX = recibir_uint8_del_buffer(unBuffer);
	contextoRecibido->r_cpu->BX = recibir_uint8_del_buffer(unBuffer);
	contextoRecibido->r_cpu->CX = recibir_uint8_del_buffer(unBuffer);
	contextoRecibido->r_cpu->DX = recibir_uint8_del_buffer(unBuffer);
	contextoRecibido->r_cpu->EAX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->EBX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->ECX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->EDX = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->SI = recibir_uint32_del_buffer(unBuffer);
	contextoRecibido->r_cpu->DI = recibir_uint32_del_buffer(unBuffer);
}

// CARGA

void cargar_generico_al_paquete(t_paquete* paquete, void* choclo, int size){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + size);
		memcpy(paquete->buffer->stream, &size, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), choclo, size);
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
												paquete->buffer->size + sizeof(int) + size);

		memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), choclo, size);
	}

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += size;
}

void cargar_int_al_paquete(t_paquete* paquete, int numero){

	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int));
		memcpy(paquete->buffer->stream, &numero, sizeof(int));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
											paquete->buffer->size + sizeof(int));
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
	}

	paquete->buffer->size += sizeof(int);
}

void cargar_char_al_paquete(t_paquete* paquete, char caracter){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(char));
		memcpy(paquete->buffer->stream, &caracter, sizeof(char));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
											paquete->buffer->size + sizeof(char));
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &caracter, sizeof(char));
	}

	paquete->buffer->size += sizeof(char);
}

void cargar_string_al_paquete(t_paquete* paquete, char* string){
	int size_string = strlen(string)+1;

	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char)*size_string);
		memcpy(paquete->buffer->stream, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char)*size_string);

	}else {
		paquete->buffer->stream = realloc(paquete->buffer->stream,
										paquete->buffer->size + sizeof(int) + sizeof(char)*size_string);
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*size_string);

	}
	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char)*size_string;
}

void cargar_uint8_al_paquete(t_paquete* un_paquete, uint8_t uint8_value){
	cargar_generico_al_paquete(un_paquete, &uint8_value, sizeof(uint8_t));
}

void cargar_uint32_al_paquete(t_paquete* un_paquete, uint32_t uint32_value){
	cargar_generico_al_paquete(un_paquete, &uint32_value, sizeof(uint32_t));
}

void cargar_size_t_al_paquete(t_paquete* un_paquete, size_t size_t_value){
	cargar_generico_al_paquete(un_paquete, &size_t_value, sizeof(size_t));
}

void agregar_registros_a_paquete(t_paquete * un_paquete, t_registrosCPU* registroRecibido){

	cargar_uint32_al_paquete(un_paquete, registroRecibido->PC);
	cargar_uint8_al_paquete(un_paquete, registroRecibido->AX);
	cargar_uint8_al_paquete(un_paquete, registroRecibido->BX);
	cargar_uint8_al_paquete(un_paquete, registroRecibido->CX);
	cargar_uint8_al_paquete(un_paquete, registroRecibido->DX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->EAX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->EBX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->ECX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->EDX);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->SI);
	cargar_uint32_al_paquete(un_paquete, registroRecibido->DI);
}

// SERVICIOS

void limpiar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void safe_free(void* elemento) {
    if (elemento != NULL) {
        free(elemento);
		elemento = NULL;
    }
}

void ejecutar_en_un_hilo_nuevo_detach(void (*f)(void*) ,void* struct_arg){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)f, struct_arg);
	pthread_detach(thread);
}

// HAY QUE AGREGAR AL SHARED
void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg){
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)f, struct_arg);
	pthread_join(thread, NULL);
}

int convertirInterfazAEnum(const char* tipo_entradasalida) {
    if (strcmp(tipo_entradasalida, "GENERICA") == 0) {
        return GENERICA;
    } else if (strcmp(tipo_entradasalida, "STDIN") == 0) {
        return STDIN;
    } else if (strcmp(tipo_entradasalida, "STDOUT") == 0) {
        return STDOUT;
    } else if (strcmp(tipo_entradasalida, "DIALFS") == 0) { // Asumo que hubo un error tipográfico en el nombre original "DIALFS"
        return DIALFS;
    } else {
        return -1;
    }
}


t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo) {
    FILE* archivo = fopen(path_archivo, "rt");
    t_list* instrucciones = list_create();
    
    if (archivo == NULL) {
        perror("No se encontró el archivo");
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(char));
    while (fgets(linea_instruccion, 256, archivo)) {
        int size_linea_actual = strlen(linea_instruccion);
        if(size_linea_actual > 2){
            if(linea_instruccion[size_linea_actual - 1] == '\n'){
                linea_instruccion[size_linea_actual - 1] = '\0';
            }
        }

        char** l_instrucciones = string_split(linea_instruccion, " ");
        int i = 0;
        while (l_instrucciones[i]) {
            i++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->pseudo_c = strdup(l_instrucciones[0]);
        pseudo_cod->fst_param = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->snd_param = (i > 2) ? strdup(l_instrucciones[2]) : NULL;
        pseudo_cod->third_param = (i > 3) ? strdup(l_instrucciones[3]) : NULL;
        pseudo_cod->fourth_param = (i > 4) ? strdup(l_instrucciones[4]) : NULL;
        pseudo_cod->fifth_param = (i > 5) ? strdup(l_instrucciones[5]) : NULL;

        char* instruccion_formateada = NULL;
        if (i == 6) {
            instruccion_formateada = string_from_format("%s %s %s %s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param, pseudo_cod->third_param, pseudo_cod->fourth_param, pseudo_cod->fifth_param);
        } else if (i == 5) {
            instruccion_formateada = string_from_format("%s %s %s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param, pseudo_cod->third_param, pseudo_cod->fourth_param);
        } else if (i == 4) {
            instruccion_formateada = string_from_format("%s %s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param, pseudo_cod->third_param);
        } else if (i == 3) {
            instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param, pseudo_cod->snd_param);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->pseudo_c, pseudo_cod->fst_param);
        } else {
            instruccion_formateada = strdup(pseudo_cod->pseudo_c);
        }

        list_add(instrucciones, instruccion_formateada);

        for (int j = 0; j < i; j++) {

			if(l_instrucciones[j] != NULL){
            	free(l_instrucciones[j]);
				l_instrucciones[j] = NULL;
			}
        }

		if(l_instrucciones != NULL){

        	free(l_instrucciones);
			l_instrucciones = NULL;
		}

		if(pseudo_cod->pseudo_c != NULL){

        	free(pseudo_cod->pseudo_c);
			pseudo_cod->pseudo_c = NULL;
		}

        if (pseudo_cod->fst_param)
		{

		 	free(pseudo_cod->fst_param);
			pseudo_cod->fst_param = NULL;
		}
        if (pseudo_cod->snd_param){
			free(pseudo_cod->snd_param);
			pseudo_cod->snd_param = NULL;
		}
			
        if (pseudo_cod->third_param){
		    free(pseudo_cod->third_param);
			pseudo_cod->third_param = NULL;
		}
			
        if (pseudo_cod->fourth_param){
		 	free(pseudo_cod->fourth_param);
			pseudo_cod->fourth_param = NULL;
		}
			
        if (pseudo_cod->fifth_param) {
			free(pseudo_cod->fifth_param);
			pseudo_cod->fifth_param = NULL;
		}

		if(pseudo_cod != NULL){
        	free(pseudo_cod);
			pseudo_cod = NULL;
		}
    }

	if(linea_instruccion != NULL){
		
    	free(linea_instruccion);
		linea_instruccion = NULL;
	}
    fclose(archivo);

    return instrucciones;
}

// DESTRUIR 

void destruir_buffer(t_buffer* buffer) {
    if (buffer == NULL) {
        return;
    }

    if (buffer->stream != NULL) {
        free(buffer->stream);
		buffer->stream = NULL;
    }
	
	if (buffer != NULL) {

    	free(buffer);
		buffer = NULL;
    }
}

void destruir_contexto_por_param(t_contexto* contexto) {
    if (contexto == NULL) {
        return; 
    }
    if (contexto->r_cpu != NULL) {
        free(contexto->r_cpu); 
		contexto->r_cpu = NULL; 
    }

	if(contexto != NULL){

    	free(contexto);
		contexto = NULL;
	}
}
