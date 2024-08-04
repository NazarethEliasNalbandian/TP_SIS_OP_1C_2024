#include "../include/servicios_memoria.h"

void retardo_respuesta(){
	usleep(RETARDO_RESPUESTA*1000);
}

void logg_crear_tabla_de_paginas(int pid, int cant_paginas){
	//PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño: <%d>", pid, cant_paginas);
}

void logg_destruir_tabla_de_paginas(int pid, int cant_paginas){
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño: <%d>", pid, cant_paginas);
}

void logg_acceso_a_tabla_de_paginas(int pid, int nro_pagina, int nro_marco){
	//PID: <PID> - Pagina: <PAGINA> - Marco: <MARCO>
	log_info(memoria_log_obligatorio, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, nro_pagina, nro_marco);
}

void logg_ampliacion_proceso(int pid, int tamanio_actual, int tamiano_a_ampliar){
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", pid, tamanio_actual, tamiano_a_ampliar);
}

void logg_reduccion_proceso(int pid, int tamanio_actual, int tamiano_a_reducir){
	log_info(memoria_log_obligatorio, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, tamanio_actual, tamiano_a_reducir);
}

void logg_acceso_a_espacio_de_usuario(int pid, char* accion, int dir_fisica, size_t tamanio){
	//PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>
	if (strcmp(accion, "leer") == 0){
		log_info(memoria_log_obligatorio, "PID: <%d> - Accion: <%s> - Direccion fisica: <%d> - Tamaño: <%zu>", pid, "LEER", dir_fisica, tamanio);
	}else if(strcmp(accion, "escribir") == 0){
		log_info(memoria_log_obligatorio, "PID: <%d> - Accion: <%s> - Direccion fisica: <%d> - Tamaño: <%zu>", pid, "ESCRIBIR", dir_fisica, tamanio);
	}else{
		log_error(memoria_log_debug, "logg_acceso_a_espacio_de_usuario, con parametro incorrecto, tiene que ser 0-lectura 1-escritura");
		exit(EXIT_FAILURE);
	}
}