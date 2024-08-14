#include "../include/cpu_memoria.h"
#include "../include/ciclo_instruccion.h"
#include "../include/servicios_cpu.h"

void atender_cpu_memoria(){
    bool control_key=1;
    while (control_key) {
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer = NULL;
		tipo_dato_parametro tipo_dato;
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_memoria,cpu_log_debug);
			destruir_buffer(unBuffer);
			break;
		case PETICION_INFO_CPU_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);
			TAM_PAGINA = recibir_int_del_buffer(unBuffer);
			destruir_buffer(unBuffer);
			break;
		case PETICION_DE_INSTRUCCIONES_CPU_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);
			char* instruccion_actual_string = recibir_string_del_buffer(unBuffer); 
			pthread_mutex_lock(&mutex_instruccion_split);

			if(instruccion_split != NULL){
				string_array_destroy(instruccion_split);
        		instruccion_split = NULL;
    		}

			sem_wait(&sem_inst);
			instruccion_split = string_split(instruccion_actual_string, " ");
			sem_post(&sem_fetch);
			destruir_buffer(unBuffer);
			pthread_mutex_unlock(&mutex_instruccion_split);
			if(instruccion_actual_string != NULL){
				free(instruccion_actual_string);
				instruccion_actual_string = NULL;
			}
			break;
		case RESIZE_CPU_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);
			rta_m_resize = recibir_string_del_buffer(unBuffer);
			destruir_buffer(unBuffer);
			sem_post(&sem_resize);
			break;
		case CONSULTA_DE_PAGINA_CPU_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			sem_wait(&sem_nro_marco);
			nro_marco = recibir_int_del_buffer(unBuffer);
			destruir_buffer(unBuffer);
			sem_post(&sem_sol_marco);
			break;
		case LECTURA_BLOQUE_CPU_MEMORIA:
			// if(valor_leido_string != NULL){
			// 	free(valor_leido_string);
			// 	valor_leido_string = NULL;
			// }
			unBuffer = recibir_paquete(fd_memoria);
			tipo_dato = recibir_int_del_buffer(unBuffer);

			if(tipo_dato == T_UINT32){
				valor_leido_uint32 = recibir_uint32_del_buffer(unBuffer);
			} else if(tipo_dato == T_UINT8)
			{
				valor_leido_uint8 = recibir_uint8_del_buffer(unBuffer);
			} else if(tipo_dato == T_STRING){
				valor_leido_string = (char*) recibir_generico_del_buffer(unBuffer);
			}
			destruir_buffer(unBuffer);
			sem_post(&sem_val_leido);
			break;
		case ESCRITURA_BLOQUE_CPU_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);
			if(valor_escrito != NULL)
			{
				free(valor_escrito);
				valor_escrito = NULL;
			}

			valor_escrito = recibir_string_del_buffer(unBuffer);
			destruir_buffer(unBuffer);
			sem_post(&sem_val_escrito);
			// free(valor_escrito);
			break;
		case -1:
			log_error(cpu_log_debug, "Desconexión de Memoria");
            control_key=0;
			break;
		default:
			log_warning(cpu_log_debug,"Operacion desconocida de Memoria.");
			break;
		}
	}
}