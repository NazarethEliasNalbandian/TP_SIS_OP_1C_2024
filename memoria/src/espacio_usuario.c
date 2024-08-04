#include "../include/espacio_usuario.h"

void escribir_uint32_en_dir_fisica(int pid, int dir_fisica, uint32_t* valor) {
    
    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + dir_fisica, valor, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);
    logg_acceso_a_espacio_de_usuario(pid, "escribir", dir_fisica, sizeof(uint32_t));
}


void escribir_uint8_en_dir_fisica(int pid, int dir_fisica, uint8_t* valor) {
    
    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + dir_fisica, valor, sizeof(uint8_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);
    logg_acceso_a_espacio_de_usuario(pid, "escribir", dir_fisica, sizeof(uint8_t));
}

void escribir_data_en_dir_fisica(int pid, int dir_fisica, void* valor, size_t tamanio) {

    if (dir_fisica < 0 || dir_fisica + tamanio > TAM_MEMORIA) {
        log_error(memoria_log_debug, "Dirección de memoria inválida: %d", dir_fisica);
        return;
    }
    
    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + dir_fisica, valor, tamanio);
    pthread_mutex_unlock(&mutex_espacio_usuario);
    logg_acceso_a_espacio_de_usuario(pid, "escribir", dir_fisica, tamanio);
}

void* leer_data_de_dir_fisica(int pid, int dir_fisica, size_t tamanio){
    void* valor_leido = malloc(tamanio);
    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, tamanio);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica, tamanio);
    return valor_leido;
}


uint32_t leer_uint32_de_dir_fisica(int pid, int dir_fisica){
    uint32_t* valor_leido = malloc(sizeof(uint32_t));
    uint32_t dato_retorno;

    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);

	dato_retorno = *valor_leido;

    if(valor_leido != NULL){
        free(valor_leido);
        valor_leido = NULL;
    }

    logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica, sizeof(uint32_t));
    return  dato_retorno;
}

uint8_t leer_uint8_de_dir_fisica(int pid, int dir_fisica){
    uint8_t* valor_leido = malloc(sizeof(uint8_t));
    uint8_t dato_retorno;

    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, sizeof(uint8_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);

    dato_retorno = *valor_leido;
    
    if(valor_leido != NULL){
        free(valor_leido);
        valor_leido = NULL;
    }

    logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica,  sizeof(uint8_t));
    return dato_retorno;
}

char* leer_string_de_dir_fisica(int pid, int dir_fisica, size_t tamanio){
    char* valor_leido = malloc(tamanio);

    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, tamanio);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    logg_acceso_a_espacio_de_usuario(pid, "leer", dir_fisica, tamanio);

    return valor_leido;
}


