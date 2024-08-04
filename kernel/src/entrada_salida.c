#include "../include/entrada_salida.h"
#include "../include/pcb.h"
#include "../include/kernel_cpu_dispatch.h"
#include "../include/servicios_kernel.h"

// FUNCIONA
bool eliminar_instancia_por_nombre(t_list* lista, char* nombre) {

    bool nombre_es_igual(void* elemento) {
        t_instancia_io* instancia = (t_instancia_io*) elemento;
        return strcmp(instancia->nombre, nombre) == 0;
    };

    t_instancia_io* instancia_eliminada = (t_instancia_io*) list_remove_by_condition(lista, (bool(*)(void*)) nombre_es_igual);
    if (instancia_eliminada != NULL) {
        free(instancia_eliminada); // Liberar la memoria del elemento eliminado
        instancia_eliminada = NULL;
        return true;
    } else {
        return false;
    }
}

// FUNCIONA
// Función que verifica si un nombre está presente en la lista de interfaces
bool existe_interfaz(char* nombre) {

    bool nombre_es_igual(void* elemento) {
        t_instancia_io* instancia = (t_instancia_io*) elemento;
        return strcmp(instancia->nombre, nombre) == 0;
    };
    
    pthread_mutex_lock(&mutex_lista_instancias_global);
    bool existe_la_interfaz = list_any_satisfy(lista_instancias_global, (bool(*)(void*)) nombre_es_igual);
    pthread_mutex_unlock(&mutex_lista_instancias_global);

    return existe_la_interfaz;
}

// FUNCIONA
t_instancia_io* obtener_instancia_por_nombre(char * nombre_interfaz){

    bool nombre_es_igual(void* elemento) {
        t_instancia_io* instancia = (t_instancia_io*) elemento;
        return strcmp(instancia->nombre, nombre_interfaz) == 0;
    };

    t_instancia_io* instancia_encontrada;

    instancia_encontrada = list_find(lista_instancias_global, (bool(*)(void*)) nombre_es_igual);

    if(instancia_encontrada == NULL)
        return NULL;

    return instancia_encontrada;
} 

// FUNCIONA
t_instancia_io* obtener_instancia_por_socket(int socket){

    bool socket_es_igual(void* elemento) {
        t_instancia_io* instancia = (t_instancia_io*) elemento;
        return instancia->socket == socket;
    };

    t_instancia_io* instancia_encontrada;
    pthread_mutex_lock(&mutex_lista_instancias_global);
    instancia_encontrada = list_find(lista_instancias_global, (bool(*)(void*)) socket_es_igual);
    pthread_mutex_unlock(&mutex_lista_instancias_global);

    if(instancia_encontrada == NULL)
    {
        log_error(kernel_log_debug,"INSTANCIA NO ENCONTRADA");
        return NULL;
    }

    return instancia_encontrada;
} 

// FUNCIONA
char* operacion_to_string(operacion_entradasalida operacion){
    switch (operacion){
        case IO_GEN_SLEEP:
            return "IO_GEN_SLEEP";
            break;
        case IO_STDIN_READ:
            return "IO_STDIN_READ";
            break;
        case IO_STDOUT_WRITE:
            return "IO_STDOUT_WRITE";
            break;
        case IO_FS_CREATE:
            return "IO_FS_CREATE";
            break;
        case IO_FS_DELETE:
            return "IO_FS_DELETE";
            break;
        case IO_FS_TRUNCATE:
            return "IO_FS_TRUNCATE";
            break;
        case IO_FS_WRITE:
            return "IO_FS_WRITE";
            break;
        case IO_FS_READ:
            return "IO_FS_READ";
            break;
        default:
            return "NO VÁLIDA";
            break;
    }
}

// FUNCIONA
bool admite_operacion_solicitada(tipo_interfaz interfaz, operacion_entradasalida operacion) {
    switch (interfaz) {
        case GENERICA:
            return operacion == IO_GEN_SLEEP;
        case STDIN:
            return operacion == IO_STDIN_READ;
        case STDOUT:
            return operacion == IO_STDOUT_WRITE;
        case DIALFS:
            return operacion == IO_FS_CREATE || operacion == IO_FS_DELETE || operacion == IO_FS_TRUNCATE || operacion == IO_FS_WRITE || operacion == IO_FS_READ;
        default:
            return false;
    }
}

void destruir_interfaz(t_instancia_io* instancia) {
    if (instancia == NULL) {
        return;
    }

    // Liberar la conexión del socket
    liberar_conexion(instancia->socket);

    // Liberar la memoria de los campos char* si no son NULL
    // if (instancia->nombre != NULL) {
    //     free(instancia->nombre);
    // }

    if (instancia->pcb_con_interfaz_asignada != NULL) {
        destruir_pcb_con_mochila(instancia->pcb_con_interfaz_asignada);
        instancia->pcb_con_interfaz_asignada = NULL; 
    }
    
    queue_destroy_and_destroy_elements(instancia->cola_de_espera, (void*) destruir_pcb_con_mochila);

    instancia->cola_de_espera = NULL;
    
    pthread_mutex_destroy(&instancia->mutex_espera);

    if(instancia != NULL){

        free(instancia);
        instancia = NULL;
    }
}

void eliminar_interfaz(t_instancia_io* instancia){


    bool nombre_es_igual(void* elemento) {
        t_instancia_io* instancia_rec = (t_instancia_io*) elemento;
        return strcmp(instancia_rec->nombre, instancia->nombre) == 0;
    };
    
    // ELIMINAR INTERFAZ DE LAS LISTAS
    pthread_mutex_lock(&mutex_lista_instancias_global);
    list_remove_and_destroy_by_condition(lista_instancias_global, (bool(*)(void*)) nombre_es_igual, (void*) destruir_interfaz);
    pthread_mutex_unlock(&mutex_lista_instancias_global);

}

void sacar_pcb_siguiente_de_la_cola_de_espera_y_mandar_a_ejecutar_entrada_salida(t_instancia_io* instancia) {
    pausador();
    if (instancia == NULL || instancia->cola_de_espera == NULL) {
        log_error(kernel_log_debug, "Instancia o cola de espera es NULL");
        return;
    }

    if (!queue_is_empty(instancia->cola_de_espera)) {
        pthread_mutex_lock(&(instancia->mutex_espera));
        t_pcb_con_mochila* pcb_con_mochila = queue_pop(instancia->cola_de_espera);
        pthread_mutex_unlock(&(instancia->mutex_espera));

        if (pcb_con_mochila == NULL) {
            log_error(kernel_log_debug, "No se pudo obtener el siguiente PCB de la cola de espera");
           if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            return;
        }
        log_trace(kernel_log_debug, "EL PCB %d SIGUE PARA EJECUTAR LA INSTANCIA: %s",pcb_con_mochila->pcb->pid , instancia->nombre);

        // if(instancia->pcb_con_interfaz_asignada!= NULL){

		// 	destruir_pcb_con_mochila(instancia->pcb_con_interfaz_asignada);
		// 	instancia->pcb_con_interfaz_asignada = NULL;
		// }

        instancia->pcb_con_interfaz_asignada = pcb_con_mochila;
        mandar_pcb_a_entrada_salida(instancia, pcb_con_mochila);
    } else {
        instancia->pcb_con_interfaz_asignada = NULL;
        instancia->libre = 1;
    }
}

// HACER FUNCIONALIDAD (CHEQUEO ENVIO DIRECCION FISICA)
void mandar_pcb_a_entrada_salida(t_instancia_io* instancia, t_pcb_con_mochila* pcb_con_mochila) {
    if (instancia == NULL || pcb_con_mochila == NULL || pcb_con_mochila->mochila == NULL) {
        log_error(kernel_log_debug, "Instancia, PCB o mochila es NULL");
        return;
    }

    t_pcb* pcb = pcb_con_mochila->pcb;
    t_mochila* mochila = pcb_con_mochila->mochila;
    size_t tamanio1 = 20;
    size_t tamanio2 = 20;
    size_t tamanio3 = 20;
    size_t tamanio4 = 20;
    size_t tamanio5 = 20;
    int unidades_trabajo = 0;
    char* nombre_archivo = NULL;
    int direccion_fisica = 0;
    int puntero_archivo = 0;
    int* puntero_puntero_archivo1 = NULL;
    int* puntero_puntero_archivo2 = NULL;
    size_t* puntero_tamanio1 = NULL;
    size_t* puntero_tamanio2 = NULL;
    size_t* puntero_tamanio3 = NULL;
    size_t* puntero_tamanio4 = NULL;
    size_t* puntero_tamanio5 = NULL;
    int* puntero_direccion_fisica1 = NULL;
    int* puntero_direccion_fisica2 = NULL;
    int* puntero_direccion_fisica3 = NULL;
    int* puntero_direccion_fisica4 = NULL;
    int* puntero_unidad_trabajo = NULL;

    switch (pcb_con_mochila->operacion_solicitada) {
        case IO_GEN_SLEEP:
            puntero_unidad_trabajo = (int*) queue_pop(mochila->parametros);
			unidades_trabajo =  *puntero_unidad_trabajo;

            if(puntero_unidad_trabajo != NULL){
			    free(puntero_unidad_trabajo);
                puntero_unidad_trabajo = NULL;
            }

            enviar_pcb_entradasalida_io_gen_sleep(pcb->pid, unidades_trabajo, instancia->socket);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            break;
        case IO_STDIN_READ:
            puntero_tamanio1 = (size_t*) queue_pop(mochila->parametros);

            if(puntero_tamanio1 != NULL){

                // memcpy(&tamanio1, puntero_tamanio1, sizeof(size_t));
                tamanio1 = *puntero_tamanio1;
                // DA INVALID FREE, LO COMENTO
			    // free(puntero_tamanio1);
                // puntero_tamanio1 = NULL;
            }
            log_trace(kernel_log_debug,"tamanio: %ld punt:%ld", tamanio1, *puntero_tamanio1);


            puntero_direccion_fisica1 = (int*) queue_pop(mochila->parametros);

            if(puntero_direccion_fisica1 != NULL){
                // DA INVALID FREE, LO COMENTO
                direccion_fisica = *puntero_direccion_fisica1;
			    // free(puntero_direccion_fisica1);
                // puntero_direccion_fisica1 = NULL;
            }


            enviar_pcb_entradasalida_io_stdin(pcb->pid, tamanio1, direccion_fisica, instancia->socket);
            if(pcb_con_mochila != NULL){

			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
	    	}
            break;
        case IO_STDOUT_WRITE:

            puntero_tamanio2 = (size_t*) queue_pop(mochila->parametros);
			tamanio2 = *puntero_tamanio2;
            
			// if(puntero_tamanio2 != NULL){

			//     // free(puntero_tamanio2);
            //     puntero_tamanio2 = NULL;
            // }

            puntero_direccion_fisica2 = (int*) queue_pop(mochila->parametros);
			direccion_fisica = *puntero_direccion_fisica2;

			// if(puntero_direccion_fisica2 != NULL){

			//     free(puntero_direccion_fisica2);
            //     puntero_direccion_fisica2 = NULL;
            // }

            enviar_pcb_entradasalida_io_stdout(pcb->pid, tamanio2, direccion_fisica, instancia->socket);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            break;
        case IO_FS_CREATE:
            nombre_archivo = (char*) queue_pop(mochila->parametros);
            enviar_pcb_entradasalida_io_fs_create(pcb->pid, nombre_archivo, instancia->socket);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }

            if(nombre_archivo != NULL){

                free(nombre_archivo);
                nombre_archivo = NULL;
            }
            break;
        case IO_FS_DELETE:
            nombre_archivo = (char*) queue_pop(mochila->parametros);
            enviar_pcb_entradasalida_io_fs_delete(pcb->pid, nombre_archivo, instancia->socket);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            if(nombre_archivo != NULL){

                free(nombre_archivo);
                nombre_archivo = NULL;
            }
            break;
        case IO_FS_TRUNCATE:
            nombre_archivo = (char*) queue_pop(mochila->parametros);

            puntero_tamanio3 = (size_t*) queue_pop(mochila->parametros);
		    tamanio3 = *puntero_tamanio3;
            
			if(puntero_tamanio3 != NULL){

			    free(puntero_tamanio3);
                puntero_tamanio3 = NULL;
            }

            enviar_pcb_entradasalida_io_fs_truncate(pcb->pid, nombre_archivo, tamanio3, instancia->socket);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }

            if(nombre_archivo != NULL){

                free(nombre_archivo);
                nombre_archivo = NULL;
            }
            break;
        case IO_FS_WRITE:
            nombre_archivo = (char*) queue_pop(mochila->parametros);

            puntero_puntero_archivo1 = (int*) queue_pop(mochila->parametros);
		    puntero_archivo = *puntero_puntero_archivo1;

            if(puntero_puntero_archivo1 != NULL){

			    free(puntero_puntero_archivo1);
                puntero_puntero_archivo1 = NULL;
            }

            puntero_tamanio4 = (size_t*) queue_pop(mochila->parametros);
			tamanio4 = *puntero_tamanio4;
            
			if(puntero_tamanio4 != NULL){

			    free(puntero_tamanio4);
                puntero_tamanio4 = NULL;
            }

            puntero_direccion_fisica3 = (int*) queue_pop(mochila->parametros);
			direccion_fisica = *puntero_direccion_fisica3;

			if(puntero_direccion_fisica3 != NULL){

			    free(puntero_direccion_fisica3);
                puntero_direccion_fisica3 = NULL;
            }


            enviar_pcb_entradasalida_io_fs_write(pcb->pid, tamanio4, direccion_fisica, instancia->socket, nombre_archivo, puntero_archivo);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }

            if(nombre_archivo != NULL){

                free(nombre_archivo);
                nombre_archivo = NULL;
            }
            break;
        case IO_FS_READ:
            nombre_archivo = (char*) queue_pop(mochila->parametros);

            puntero_tamanio5 = (size_t*) queue_pop(mochila->parametros);
			tamanio5 = *puntero_tamanio5;
			if(puntero_tamanio5 != NULL){

			    free(puntero_tamanio5);
                puntero_tamanio5 = NULL;
            }

            puntero_direccion_fisica4 = (int*) queue_pop(mochila->parametros);
			direccion_fisica = *puntero_direccion_fisica4;
			if(puntero_direccion_fisica4 != NULL){

			    free(puntero_direccion_fisica4);
                puntero_direccion_fisica4 = NULL;
            }

            puntero_puntero_archivo2 = (int*) queue_pop(mochila->parametros);
			puntero_archivo = *puntero_puntero_archivo2;
			if(puntero_puntero_archivo2 != NULL){

			    free(puntero_puntero_archivo2);
                puntero_puntero_archivo2 = NULL;
            }

            enviar_pcb_entradasalida_io_fs_read(pcb->pid, tamanio5, puntero_archivo, direccion_fisica, instancia->socket, nombre_archivo);
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            if(nombre_archivo != NULL){

                free(nombre_archivo);
                nombre_archivo = NULL;
            }
            break;
        default:
            log_info(kernel_log_debug, "OPERACION DE ENTRADA SALIDA NO RECONOCIDA");
            if(pcb_con_mochila != NULL){
			    destruir_pcb_con_mochila(pcb_con_mochila);
			    pcb_con_mochila = NULL;
		    }
            break;
    }


}
