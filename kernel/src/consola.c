#include "../include/consola.h"
#include "../include/planificador_largo_plazo.h"
#include "../include/pcb.h"
#include "../include/servicios_kernel.h"

void _destruir_elementos(t_instruction* instruction){

	if(instruction->instruction_name != NULL){

		free(instruction->instruction_name);
		instruction->instruction_name = NULL;

	}

	if(instruction != NULL){

		free(instruction);
		instruction = NULL;
	}
}

void add_instruction_list(t_list* lista, char* instruccion, t_op_instruction op_instruction_code, int n_param){
	t_instruction* instruction = malloc(sizeof(t_instruction));
	instruction->instruction_name = malloc(sizeof(char)*(strlen(instruccion) + 1));
	memcpy(instruction->instruction_name, instruccion, sizeof(char)*(strlen(instruccion) + 1));
	instruction->op_instruction = op_instruction_code;
	instruction->instruction_n_param = n_param;

	list_add(lista, instruction);
}

void _inicializar_consola(){

	lista_instructions = list_create();
	pthread_mutex_lock(&mutex_lista_instrucciones);
    add_instruction_list(lista_instructions, "EJECUTAR_SCRIPT", EJECUTAR_SCRIPT, 1);
	add_instruction_list(lista_instructions, "INICIAR_PROCESO", INICIAR_PROCESO, 1);
	add_instruction_list(lista_instructions, "FINALIZAR_PROCESO", FINALIZAR_PROCESO, 1);
	add_instruction_list(lista_instructions, "DETENER_PLANIFICACION", DETENER_PLANIFICACION, 0);
	add_instruction_list(lista_instructions, "INICIAR_PLANIFICACION", INICIAR_PLANIFICACION, 0);
	add_instruction_list(lista_instructions, "MULTIPROGRAMACION", MULTIPROGRAMACION, 1);
	add_instruction_list(lista_instructions, "PROCESO_ESTADO", PROCESO_ESTADO, 0);
	pthread_mutex_unlock(&mutex_lista_instrucciones);
}

bool _validar_instruccion(char* leido){
	bool resultado_validacion = false;

	char** comando_consola = string_split(leido, " ");

	if((strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "INICIAR_PROCESO") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0))
		resultado_validacion = true;
	else if ((strcmp(comando_consola[0], "PROCESO_ESTADO") == 0))
		resultado_validacion = true;
	else
		log_error(kernel_log_debug, "Comando no reconocido");

	string_array_destroy(comando_consola);

	return resultado_validacion;
}

void _pausar_planificadores(){
	if(var_pausa == 1){
		log_info(kernel_log_debug, "Los planificadores ya se encuentran pausados");
	}else{
		var_pausa = 1;
		pausador();
	}
}

void _finalizar_proceso_por_PID(char* un_pid){
	int pid = atoi(un_pid);
	plp_planificar_proceso_exit(pid);

	if(un_pid != NULL){

		free(un_pid);
		un_pid = NULL;
	}
}


void _iniciar_planificadores(){
	if(var_pausa == 0){
//		El enunciado dice que se debe ignorar el mensaje en caso
//		de q' los planificadores no se encuentren pausados
	}else{
		var_pausa = 0;
		log_info(kernel_log_debug, "INICIO DE PLANIFICACIÓN");
		sem_post(&sem_pausa);
	}
}

void _cambiar_grado_de_multiprogramacion(char* un_valor){
	int nuevo_valor = atoi(un_valor);
	int valor_anterior = GRADO_MULTIPROGRAMACION;
	int aumento_grado_multiprogramacion = nuevo_valor - GRADO_MULTIPROGRAMACION;
	int disminucion_grado_multiprogramacion = (-1) * aumento_grado_multiprogramacion;
	int i;

	if(nuevo_valor >= 1){

		if(aumento_grado_multiprogramacion >=0){
			for(i=0; i < aumento_grado_multiprogramacion; i++){
				sem_post(&sem_procesos_en_memoria);
			}
		} else
		{
			for(i=0; i < disminucion_grado_multiprogramacion; i++){
				sem_wait(&sem_procesos_en_memoria);
			}
		}

		GRADO_MULTIPROGRAMACION = nuevo_valor;
		log_info(kernel_log_obligatorio, "Grado Anterior: %d - Grado Actual: %d" ,valor_anterior ,nuevo_valor);
    
	}else{
		log_error(kernel_log_debug, "EL grado de multiprogramacion tiene que ser >= 1");
	}
	
	if(un_valor != NULL){

		free(un_valor);
		un_valor = NULL;
	}
}

void imprimir_procesos_por_estado(){

	int* __encontrando_el_maximo(int* valor_1, int* valor_2){
		if(*valor_1 >= *valor_2)return valor_1;
		else return valor_2;
	}

	int* mayor_size;
	int size_new = list_size(lista_new);
	int size_ready = list_size(lista_ready);
	int size_execute = list_size(lista_execute);
	int size_blocked = list_size(lista_blocked);
	int size_exit = list_size(lista_exit);
	int size_ready_prioridad = list_size(lista_ready_prioridad);

	t_list* una_lista_12345 = list_create();
	list_add(una_lista_12345, &size_new);
	list_add(una_lista_12345, &size_ready);
	list_add(una_lista_12345, &size_execute);
	list_add(una_lista_12345, &size_blocked);
	list_add(una_lista_12345, &size_exit);
	list_add(una_lista_12345, &size_ready_prioridad);
	mayor_size = (int*)list_get_maximum(una_lista_12345, (void*)__encontrando_el_maximo);

	int ancho_columna = 16;
	char* borde_completo = string_repeat('-', ancho_columna*5);

	char* header = string_new();
	string_append(&header, " |===== NEW ====");
	string_append(&header, "=|==== READY ===");
	string_append(&header, "=|=== EXECUTE ==");
	string_append(&header, "=|=== BLOCKED ==");
	string_append(&header, "=|==== EXIT ====");
	string_append(&header, "=|== READY PR ==");
		 char* relleno_1 = " |              ";
//		 char* relleno_2 = "=|==============";

	char* relleno_caracteres = string_repeat('-', 69);
	printf(" [GMMP: %d] %s\n", GRADO_MULTIPROGRAMACION, relleno_caracteres);
	printf("%s\n", header);

//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) ");
//	printf(" | PID: 001 (2) \n");

//----------------------------------------------------------
	char* __traer_string_con_data(t_list* una_lista, int elemento, int size_r){
		char* respuesta_char = string_new();
		char* aux_1 = string_new();
		char* aux_2;
		char* ultimos_3;
		string_append(&respuesta_char, " | PID: ");
//		t_pcb* un_pcb = list_get(una_lista, elemento); //<
		t_pcb* un_pcb = list_get(una_lista, size_r - (elemento + 1)); //<
		aux_2 = string_itoa(un_pcb->pid);
		string_append(&aux_1, "00");
		string_append(&aux_1, aux_2); //[001|0032|00076|000981]
		int size = strlen(aux_1);
		if(size >= 3){
			ultimos_3 = malloc(4*sizeof(char));
			memcpy(ultimos_3, aux_1 + size - 3, 4*sizeof(char));
		}else{
			log_error(kernel_log_debug, "El size deberia se >= 3");
			exit(EXIT_FAILURE);
		}
		string_append(&respuesta_char, ultimos_3);

		free(aux_2);//[12]
		free(aux_1);//[00] -> [0012]
		free(ultimos_3);//[012]
		return respuesta_char;
	}
//----------------------------------------------------------
	char* __imprimir_linea_completa(int nro_linea){
		char* linea = string_new();
		char* char_aux;

		//Evaluando lista_new
		if(size_new > nro_linea){
			char_aux = __traer_string_con_data(lista_new, nro_linea, size_new);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_ready
		if(size_ready > nro_linea){
			char_aux = __traer_string_con_data(lista_ready, nro_linea, size_ready);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_execute
		if(size_execute > nro_linea){
			char_aux = __traer_string_con_data(lista_execute, nro_linea, size_execute);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_blocked
		if(size_blocked > nro_linea){
			char_aux = __traer_string_con_data(lista_blocked, nro_linea, size_blocked);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}

		//Evaluando lista_exit
		if(size_exit > nro_linea){
			char_aux = __traer_string_con_data(lista_exit, nro_linea, size_exit);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}
		//Evaluando lista_ready_prioridad
		if(size_ready_prioridad > nro_linea){
			char_aux = __traer_string_con_data(lista_ready_prioridad, nro_linea, size_ready_prioridad);
			string_append(&linea, char_aux);
			free(char_aux);
		}else{
			string_append(&linea, relleno_1);
		}


		return linea;
	}
//----------------------------------------------------------

	int contador_linea = 0;

	while(*mayor_size > contador_linea){
		char* linea = __imprimir_linea_completa(contador_linea);
		printf("%s\n", linea);
		contador_linea++;
		free(linea);
	}


	printf("%s\n", borde_completo);

//	char* prueba_1;
//	char* prueba_2;

	free(relleno_caracteres);
	free(borde_completo);
	free(header);
	list_destroy(una_lista_12345);

}

void _atender_instruccion_validada(char* leido){
	char** comando_consola = string_split(leido, " ");

	//Obtener el componente de la instruccion respectiva
	bool __buscar_instruccion(t_instruction* instruction){
		if(strcmp(instruction->instruction_name, comando_consola[0]) == 0){
			return true;
		}
		return false;
	};

	pthread_mutex_lock(&mutex_lista_instrucciones);
	t_instruction* instruction = list_find(lista_instructions, (void*)__buscar_instruccion);
	pthread_mutex_unlock(&mutex_lista_instrucciones);
	if (instruction == NULL) {
		log_error(kernel_log_debug, "Instrucción no encontrada: %s", comando_consola[0]);
		string_array_destroy(comando_consola);
		return;
	}

	//Evaluar en el SWITCH CASE
	switch (instruction->op_instruction) {
		case EJECUTAR_SCRIPT: // [PATH]
            char* path = string_duplicate(comando_consola[1]);
            ejecutar_instrucciones_desde_archivo(path);
			if(path != NULL){
			
            	free(path);
				path = NULL;
			}
			
            break;
		case INICIAR_PROCESO: // [PATH]
			t_pcb* un_pcb = crear_pcb(comando_consola[1]);
			// imprimir_pcb(un_pcb);
			ejecutar_en_un_hilo_nuevo_detach((void*)plp_planificar_proceso_nuevo, un_pcb);
			break;
		case FINALIZAR_PROCESO: // [PID]
			char* copia_del_pid = string_duplicate(comando_consola[1]);
			ejecutar_en_un_hilo_nuevo_detach((void*)_finalizar_proceso_por_PID, copia_del_pid);
			break;
		case DETENER_PLANIFICACION: //_none
			ejecutar_en_un_hilo_nuevo_detach((void*)_pausar_planificadores, NULL);
			break;
		case INICIAR_PLANIFICACION: //_none
			ejecutar_en_un_hilo_nuevo_detach((void*)_iniciar_planificadores, NULL);
			break;
		case MULTIPROGRAMACION: // [int]
			char* copia_del_GMMP = string_duplicate(comando_consola[1]);
			ejecutar_en_un_hilo_nuevo_detach((void*)_cambiar_grado_de_multiprogramacion, copia_del_GMMP);
			break;
		case PROCESO_ESTADO: //_none
			imprimir_procesos_por_estado();
			break;
		default:
			break;
	}

	string_array_destroy(comando_consola);
}

// void iniciar_consola_interactiva(){
// 	char* leido;

// 	leido = readline("> ");
// 	bool validacion_leido;

// 	while(strcmp(leido, "\0") != 0){
// 		validacion_leido = _validar_instruccion(leido);

// 		if(!validacion_leido){
// 			log_error(kernel_logger, "COMANDO NO RECONOCIDO");
// 			free(leido);
// 			leido = readline("> ");
// 			continue;
// 		}

// 		_atender_instruccion_validada(leido);
// 		free(leido);
// 		leido = readline("> ");
// 	}
// 	free(leido);
// }


void _leer_comandos(){
	//validaciones - El comando y la cantidad de parametros por consola sea el correcto
	char* leido;
	// leido = readline("> ");
	// while(strcmp(leido,"\0") != 0){
	// 	if(_validar_instruccion(leido)){
	// 		printf("Comando válido\n");
	// 		_atender_instruccion_validada(leido);
	// 	}
	// 	free(leido);
	// 	leido = readline("> ");
	// }
	// free(leido);


	while(strcmp(leido= readline(">"),"")!=0){

		if(!strcmp(leido,"listo"))
				break;

		if(_validar_instruccion(leido)){
			printf("Comando válido\n");
			_atender_instruccion_validada(leido);
		}
		if(leido != NULL){

			free(leido);
			leido = NULL;
		}

	};
	if(leido != NULL){

		free(leido);
		leido = NULL;
	}

	// clear_history();
	// rl_clear_history();
}

// void _leer_comandos() {
//     char* leido;

//     while ((leido = readline("> ")) != NULL) {
//         // Verificar si la entrada no está vacía
//         if (strlen(leido) > 0) {
//             if (_validar_instruccion(leido)) {
//                 printf("Comando válido\n");
//                 _atender_instruccion_validada(leido);
//             } else {
//                 printf("Comando inválido\n");
//             }
//         }
//         free(leido);  // Liberar la memoria asignada por readline
//     }

//     // Limpiar recursos de readline
//     rl_cleanup_after_signal();
//     rl_clear_history();
// }

void _finalizar_consola() {
	pthread_mutex_lock(&mutex_lista_instrucciones);
    if (lista_instructions != NULL) {
        list_destroy_and_destroy_elements(lista_instructions, (void *)_destruir_elementos);
        lista_instructions = NULL; 
    }
	pthread_mutex_unlock(&mutex_lista_instrucciones);
}

void _leer_consola(){

	_inicializar_consola();

    pthread_t hilo_consola;
	pthread_create(&hilo_consola, NULL, (void*)_leer_comandos, NULL);
	pthread_join(hilo_consola, NULL);

	_finalizar_consola();
}

void ejecutar_instrucciones_desde_archivo(char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        log_error(kernel_log_debug,"No se pudo abrir el archivo de comandos");
        return;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        // Eliminar el salto de línea al final de la línea si es necesario
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        _atender_instruccion_validada(line);
    }

	if(line != NULL)
	{
		free(line);
		line = NULL;
	}

    fclose(file);
}
