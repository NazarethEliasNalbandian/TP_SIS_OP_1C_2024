#include "../include/fcb.h"
#include "../include/bloques.h"


void inicializar_fcb(char* nombre_archivo) {
    size_t path_len = strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo) + 1;
    char* path_archivo = malloc(path_len);
    if (path_archivo == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para path_archivo");
        return;
    }

    strcpy(path_archivo, PATH_BASE_DIALFS);
    strcat(path_archivo, nombre_archivo);

    // Creo el FCB y lo agrego a la lista de structs
    t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));
    if (nuevo_fcb == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para nuevo_fcb");
        if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
        }
        return;
    }

    nuevo_fcb->nombre = malloc(strlen(nombre_archivo) + 1);
    if (nuevo_fcb->nombre == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para nuevo_fcb->nombre");

        if(nuevo_fcb != NULL){

            free(nuevo_fcb);
            nuevo_fcb = NULL;
        }

        if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
        }

        return;
    }
    strcpy(nuevo_fcb->nombre, nombre_archivo);

    nuevo_fcb->tamanio = 0;
    nuevo_fcb->bloque_inicial = -1;
    nuevo_fcb->tamanio_en_bloques = 1;

    char text_tamanio_archivo[10];
    sprintf(text_tamanio_archivo, "%ld", nuevo_fcb->tamanio);

    list_add(lista_fat, nuevo_fcb);

    nuevo_fcb->archivo_metadata = config_create(path_archivo);
    if (nuevo_fcb->archivo_metadata == NULL) {
        FILE* file_fcb = fopen(path_archivo, "a+");
        if (file_fcb == NULL) {
            log_error(io_log_debug, "No se pudo crear el archivo de metadata");

            if(nuevo_fcb->nombre != NULL){

                free(nuevo_fcb->nombre);
                nuevo_fcb->nombre = NULL;
            }

            if(nuevo_fcb != NULL){

                free(nuevo_fcb);
                nuevo_fcb = NULL;
            }

            if(path_archivo != NULL){

                free(path_archivo);
                path_archivo = NULL;
            }
            return;
        }
        fclose(file_fcb);

        nuevo_fcb->archivo_metadata = config_create(path_archivo);
        if (nuevo_fcb->archivo_metadata == NULL) {
            log_error(io_log_debug, "No se pudo crear el archivo de configuración");
            if(nuevo_fcb->nombre != NULL){
                free(nuevo_fcb->nombre);
                nuevo_fcb->nombre = NULL;
            }

            if(nuevo_fcb != NULL){
                free(nuevo_fcb);
                nuevo_fcb = NULL;
            }

            if(path_archivo != NULL){
                free(path_archivo);
                path_archivo = NULL;
            }
            return;
        }
    }

    config_set_value(nuevo_fcb->archivo_metadata, "BLOQUE_INICIAL", "NULL");
    config_set_value(nuevo_fcb->archivo_metadata, "TAMANIO_ARCHIVO", text_tamanio_archivo);

    config_save(nuevo_fcb->archivo_metadata);

    if(path_archivo != NULL){
        free(path_archivo);
        path_archivo = NULL;
    }
}

void crear_fcb(char* nombre_archivo) {
    size_t path_len = strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo) + 1;
    char* path_archivo = malloc(path_len);
    if (path_archivo == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para path_archivo");
        return;
    }

    strcpy(path_archivo, PATH_BASE_DIALFS);
    strcat(path_archivo, nombre_archivo);

    // Creo el FCB y lo agrego a la lista de structs
    t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));
    if (nuevo_fcb == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para nuevo_fcb");

        if(path_archivo != NULL){

            free(path_archivo);
            path_archivo = NULL;
        }
        return;
    }

    nuevo_fcb->nombre = malloc(strlen(nombre_archivo) + 1);
    if (nuevo_fcb->nombre == NULL) {
        log_error(io_log_debug, "No se pudo asignar memoria para nuevo_fcb->nombre");
        if(nuevo_fcb != NULL){
            free(nuevo_fcb);
            nuevo_fcb = NULL;
        }

        if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
        }
        return;
    }

    nuevo_fcb->archivo_metadata = config_create(path_archivo);
    nuevo_fcb->bloque_inicial = config_get_int_value(nuevo_fcb->archivo_metadata, "BLOQUE_INICIAL");
    nuevo_fcb->tamanio =  config_get_int_value(nuevo_fcb->archivo_metadata, "TAMANIO_ARCHIVO");

    strcpy(nuevo_fcb->nombre, nombre_archivo);

    nuevo_fcb->tamanio_en_bloques = tamanio_en_bloques(nuevo_fcb->tamanio);

    list_add(lista_fat, nuevo_fcb);

    if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
    }
}

t_fcb* obtener_fcb(char* nombre_archivo) {
    t_fcb* fcb_buscado = NULL;

    if (!list_is_empty(lista_fat)) {
        for (int i = 0; i < list_size(lista_fat); i++) {
            fcb_buscado = list_get(lista_fat, i);
            if (fcb_buscado != NULL && strcmp(fcb_buscado->nombre, nombre_archivo) == 0) {
                pthread_mutex_unlock(&mutex_lista_fat);
                return fcb_buscado;
            }
        }
    }
    return NULL;
}

// USAR CON MUTEX
void setear_valor_entero_en_fcb(t_fcb* una_fcb, char* clave, int valor) {
    if (una_fcb == NULL || clave == NULL) {
        log_error(io_log_debug, "Error: FCB o clave nulo en setear_valor_entero_en_fcb");
        return;
    }

    char* text_valor = malloc(10);
    if (text_valor == NULL) {
        log_error(io_log_debug, "Error: No se pudo asignar memoria para text_valor");
        return;
    }

    sprintf(text_valor, "%d", valor);

    for (int i = 0; i < list_size(lista_fat); i++) {
        t_fcb* archivo_buscado = list_get(lista_fat, i);
        if (archivo_buscado != NULL && strcmp(archivo_buscado->nombre, una_fcb->nombre) == 0) {
            
            config_set_value(archivo_buscado->archivo_metadata, clave, text_valor);
            config_save(archivo_buscado->archivo_metadata);
            break;
        }
    }

    if(text_valor != NULL){

        free(text_valor);
        text_valor = NULL;
    }
}

void setear_valor_string_en_fcb(t_fcb* una_fcb, char* clave, char* valor){
	//nomrbe_De_archivo.fcb <- Setear en el archivo concreto
	//Se puede setear el size de la lista de fcbs aca o afuera en la logica general

	char* text_valor = malloc(10);
	sprintf(text_valor, "%s", valor);

	for(int i = 0; i < list_size(lista_fat); i++){
		t_fcb* archivo_buscado = list_get(lista_fat, i);
		if(strcmp(archivo_buscado->nombre, una_fcb->nombre) == 0){
			config_set_value(archivo_buscado->archivo_metadata, clave, text_valor);
			config_save(archivo_buscado->archivo_metadata);
		}
	}

}

void finalizar_fcb(t_fcb* fcb){
    if (fcb == NULL) {
        log_error(io_log_debug,"ARCHIVO A ELIMINAR ES NULO");
        return;
    }

    if (fcb->archivo_metadata != NULL) {
        // Guardar el path del archivo antes de destruir la configuración
        char* path_archivo = strdup(fcb->archivo_metadata->path);

        // Destruir la configuración
        config_destroy(fcb->archivo_metadata);

        if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
        }
    }

    if(fcb->nombre != NULL){

	    free(fcb->nombre);
        fcb->nombre = NULL;
    }
    if(fcb != NULL){

	    free(fcb);
        fcb = NULL;
    }
}

void destruir_fcb(t_fcb* fcb){
    if (fcb == NULL) {
        log_error(io_log_debug,"ARCHIVO A ELIMINAR ES NULO");
        return;
    }

    if (fcb->archivo_metadata != NULL) {
        // Guardar el path del archivo antes de destruir la configuración
        char* path_archivo = strdup(fcb->archivo_metadata->path);

        // Destruir la configuración
        config_destroy(fcb->archivo_metadata);

        // Eliminar el archivo del sistema de archivos
        if (remove(path_archivo) != 0) {
            log_error(io_log_debug, "No se pudo eliminar el archivo de metadata");
        } else {
            log_info(io_log_debug, "Archivo de metadata eliminado correctamente");
        }

        if(path_archivo != NULL){
            free(path_archivo);
            path_archivo = NULL;
        }
    }

    if(fcb->nombre != NULL){

	    free(fcb->nombre);
        fcb->nombre = NULL;
    }
    
    if(fcb != NULL){

	    free(fcb);
        fcb = NULL;
    }
}

void destruir_listas_fcbs(){
	list_destroy_and_destroy_elements(lista_fat, (void*) finalizar_fcb);
}

void eliminar_fcb_de_fat(char* nombre_archivo){
	bool comparar_nombre_fcb(void* fcb) {
        return strcmp(((t_fcb*)fcb)->nombre, nombre_archivo) == 0;
    };

    list_remove_and_destroy_by_condition(lista_fat, comparar_nombre_fcb , (void*) destruir_fcb);
}