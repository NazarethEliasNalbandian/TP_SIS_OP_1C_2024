#include "../include/servicios_cpu.h"
#include "../include/ciclo_instruccion.h"
#include "../include/cpu_kernel_dispatch.h"

nombre_instruccion_comando convertirStringAEnum(char* nombre_instruccion){
    if(strcmp(nombre_instruccion,"SET") == 0)
        return SET;
    else if(strcmp(nombre_instruccion,"SUM") == 0)
        return SUM;
    else if(strcmp(nombre_instruccion,"SUB") == 0)
        return SUB;
    else if(strcmp(nombre_instruccion,"JNZ") == 0)
        return JNZ;
    else if(strcmp(nombre_instruccion,"IO_GEN_SLEEP") == 0)
        return IO_GEN_SLEEP;
    else if(strcmp(nombre_instruccion,"MOV_IN") == 0)
        return MOV_IN;
    else if(strcmp(nombre_instruccion,"MOV_OUT") == 0)
        return MOV_OUT;
    else if(strcmp(nombre_instruccion,"RESIZE") == 0)
        return RESIZE;
    else if(strcmp(nombre_instruccion,"WAIT") == 0)
        return WAIT;
    else if(strcmp(nombre_instruccion,"SIGNAL") == 0)
        return SIGNAL;
    else if(strcmp(nombre_instruccion,"COPY_STRING") == 0)
        return COPY_STRING;
    else if(strcmp(nombre_instruccion,"IO_STDIN_READ") == 0)
        return IO_STDIN_READ;
    else if(strcmp(nombre_instruccion,"IO_STDOUT_WRITE") == 0)
        return IO_STDOUT_WRITE;
    else if(strcmp(nombre_instruccion,"IO_FS_CREATE") == 0)
        return IO_FS_CREATE;
    else if(strcmp(nombre_instruccion,"IO_FS_DELETE") == 0)
        return IO_FS_DELETE;
    else if(strcmp(nombre_instruccion,"IO_FS_TRUNCATE") == 0)
        return IO_FS_TRUNCATE;
    else if(strcmp(nombre_instruccion,"IO_FS_READ") == 0)
        return IO_FS_READ;
    else if(strcmp(nombre_instruccion,"IO_FS_WRITE") == 0)
        return IO_FS_WRITE;
    else if(strcmp(nombre_instruccion,"EXIT") == 0)
        return EXITT;
    else    
        return -1;
}

bool validador_de_header(char* header_string){
	// log_info(cpu_logger, "String a evaluar: %s", header_string);
	bool respuesta = false;
	int i = 0;
	while(op_autorizada[i] != NULL){
		if(strcmp(op_autorizada[i], header_string) == 0) 
            respuesta = true;
		i++;
	}
	// log_info(cpu_logger, "Valor del bool: %d", respuesta);
	return respuesta;
}

void enviar_contexto_a_kernel_con_desalojo_cpu(){
    desalojar=true;
    t_paquete* un_paquete = crear_paquete(tipo_desalojo);
    agregar_contexto_a_paquete(un_paquete);
    enviar_paquete(un_paquete, fd_kernel_dispatch);
    eliminar_paquete(un_paquete);

    if(tipo_desalojo == ATENDER_EXIT){
        log_trace(cpu_log_debug, "ENVIO DE PID <%d> A KERNEL", un_contexto->pID);
    }
    else 
        log_error(cpu_log_debug, "ENVIO DE PID <%d> A KERNEL", un_contexto->pID);
}

void agregar_contexto_a_paquete(t_paquete * un_paquete){

	cargar_int_al_paquete(un_paquete, un_contexto->pID);
	cargar_int_al_paquete(un_paquete, un_contexto->verificador);
	agregar_registros_a_paquete(un_paquete, un_contexto->r_cpu);
}

bool hay_io(){
    switch(nombre_instruccion_enum)
    {
        case IO_FS_CREATE:
        case IO_FS_DELETE:
        case IO_FS_TRUNCATE:
        case IO_FS_WRITE:
        case IO_FS_READ:
        case IO_GEN_SLEEP:
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:
            return true;
        default:
            return false;
    }
}

bool es_registro_uint32(char* registro){
    if(string_starts_with(registro, "E") || (strcmp(registro,"PC") == 0)  || (strcmp(registro,"SI") == 0) || (strcmp(registro,"DI") == 0))
        return true;
    else    
        return false;
}

bool es_registro_uint8(char* registro){
    if((strcmp(registro,"AX") == 0)  || (strcmp(registro,"BX") == 0) || (strcmp(registro,"CX") == 0) || (strcmp(registro,"DX") == 0))
        return true;
    else    
        return false;
}

void asignarRegistroExtended(char* registro, uint32_t** valor_a_guardar)
{
    if(es_registro_uint32(registro)){
        *valor_a_guardar = detectar_registroE(registro);
    } 
}

void asignarRegistro(char* registro, uint8_t** valor_a_guardar)
{
    if(es_registro_uint8(registro)){
        *valor_a_guardar = detectar_registro(registro);
    } 
}

int obtenerIntDeRegistro(char* registro, uint32_t** er, uint8_t**r){
                
    if(es_registro_uint32(registro)){
        asignarRegistroExtended(registro, er);
        return ((int)(**er));
    }else if(es_registro_uint8(registro)){
        asignarRegistro(registro, r);
        return ((int)(**r));
    } else{
        log_error(cpu_log_debug, "REGISTRO INVALIDO: %s", instruccion_split[3]);
        exit(EXIT_FAILURE);
    }
}

u_int32_t obtenerUint32DeRegistro(char* registro, uint32_t** er){
                
    asignarRegistroExtended(registro, er);
    return ((uint32_t)(**er));
    
}

u_int8_t obtenerUint8DeRegistro(char* registro, uint8_t**r){
                
    asignarRegistro(registro, r);
    return ((uint8_t)(**r));
}

size_t obtenerSizeTDeRegistro(char* registro, uint32_t** er, uint8_t**r){
                
    if(es_registro_uint32(registro)){
        asignarRegistroExtended(registro, er);
        return ((size_t)(**er));
    }else if(es_registro_uint8(registro)){
        asignarRegistro(registro, r);
        return ((size_t)(**r));
    } else{
        log_error(cpu_log_debug, "REGISTRO INVALIDO: %s", instruccion_split[3]);
        exit(EXIT_FAILURE);
    }
}

void escribir_registro(char* registro_a_escribir,int valor, uint32_t* registroAEscribirUint32, uint8_t* registroAEscribirUint8){
    if(es_registro_uint32(registro_a_escribir)){
        registroAEscribirUint32 = detectar_registroE(registro_a_escribir);
        *registroAEscribirUint32 = (uint32_t) valor;

    }else if(es_registro_uint8(registro_a_escribir)){
        registroAEscribirUint8 = detectar_registro(registro_a_escribir);
        *registroAEscribirUint8 = (uint8_t) valor;
    }else{
        log_error(cpu_log_debug, "REGISTRO A ESCRIBIR NO RECONOCIDO");
        exit(EXIT_FAILURE);
    }
}

void liberar_instruccion_split(char** instruccion_split) {
    if (instruccion_split == NULL) {
        return;
    }

    // // Asumiendo que la matriz de cadenas termina en NULL
    // for (int i = 0; instruccion_split[i] != NULL; i++) {
    //     free(instruccion_split[i]);
    //     instruccion_split[i] = NULL; // Liberar cada cadena individual
    // }

    if(instruccion_split != NULL){

        free(instruccion_split);
        instruccion_split = NULL;
    }
}