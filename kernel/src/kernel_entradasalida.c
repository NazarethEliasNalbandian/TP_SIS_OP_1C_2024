#include "../include/kernel_entradasalida.h"
#include "../include/pcb.h"
#include "../include/entrada_salida.h"
#include "../include/servicios_kernel.h"
#include "../include/planificador_largo_plazo.h"
#include "../include/planificador_corto_plazo.h"

void atender_kernel_entradasalida(void* arg){
	
	t_instancia_args* args = (t_instancia_args*) arg;
	int socket = args->socket;

	if(args != NULL){

		free(args);
		args = NULL;
	}
	t_instancia_io* instancia;

	bool control_key = 1;
	while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(socket);
		switch (cod_op) {
			case NOMBRE_ENTRADA_SALIDA:
				instancia = malloc(sizeof(t_instancia_io));
				unBuffer = __recibiendo_super_paquete(socket);
				asignar_nombre_y_tipo_y_agregar_a_listas(unBuffer, instancia, socket);
				break;

			case FIN_ENTRADASALIDA:
				unBuffer = __recibiendo_super_paquete(socket);
				instancia = obtener_instancia_por_socket(socket);
				atender_fin_entradasalida(unBuffer, instancia);
				break;
			
			case -1:
				log_error(kernel_log_debug, "DESCONEXION DE %s", instancia->nombre);
				instancia = obtener_instancia_por_socket(socket);
				eliminar_interfaz(instancia);
				control_key = 0;
				break;
				
			default:
				log_warning(kernel_log_debug,"OPERACION DESCONOCIDA DE %s", instancia->nombre);
				break;
		}
		
	}
}

void asignar_nombre_y_tipo_y_agregar_a_listas(t_buffer* unBuffer, t_instancia_io* nueva_instancia, int fd_entradasalida){

    nueva_instancia->socket = fd_entradasalida;
    nueva_instancia->libre = 1;
	nueva_instancia->cola_de_espera = queue_create(); 
	nueva_instancia->pcb_con_interfaz_asignada = NULL;

	pthread_mutex_init(&(nueva_instancia->mutex_espera),NULL);
    char* nombre_entradasalida = __recibir_string_del_buffer(unBuffer);
    int tipo_io = __recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	nueva_instancia->nombre = strdup(nombre_entradasalida);
	nueva_instancia->tipo = tipo_io;

	pthread_mutex_lock(&mutex_lista_instancias_global);
	list_add(lista_instancias_global, nueva_instancia);
	pthread_mutex_unlock(&mutex_lista_instancias_global);

	log_info(kernel_log_debug, "Nueva instancia de E/S conectada: %s y SOCKET: %d", nueva_instancia->nombre, nueva_instancia->socket);

	if(nombre_entradasalida != NULL){

		free(nombre_entradasalida);
		nombre_entradasalida = NULL;
	}
}

void atender_fin_entradasalida(t_buffer* unBuffer, t_instancia_io* instancia){
	int pid = __recibir_int_del_buffer(unBuffer);
	char* resultado = __recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	pausador();

	log_info(kernel_log_debug, "PID: %d Y RESULTADO: %s", pid, resultado);

	if(strcmp(resultado,"OK") == 0){
		// Obtenemos el PCB a traves del id
		// Sacamos el PCB de la lista de bloqueados general
		t_pcb* pcb = buscar_y_remover_pcb_por_pid(pid);

		// Sacamos el siguiente PCB en espera para utilizar la entrada salida y lo mandamos a la entrada salida
		sacar_pcb_siguiente_de_la_cola_de_espera_y_mandar_a_ejecutar_entrada_salida(instancia);

		// Agrego el PCB a la lista Ready
		analizarSiLoMandoAReadyOReadyPrioridad(pcb);
		
		// Replanifico
		pcp_planificar_corto_plazo();
	} else{
		t_pcb* pcb = buscar_pcb_por_pid_en(pid, lista_blocked);
		plp_exit(pcb);
	}

	if(resultado != NULL){

		free(resultado);
		resultado = NULL;
	}
}
