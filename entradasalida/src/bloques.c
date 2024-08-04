#include "../include/bloques.h"
#include "../include/bitmap.h"
#include "../include/fcb.h"

//========== BLOQUES =====================

int tamanio_en_bloques(int tamanio_en_bytes){
    if(tamanio_en_bytes == 0)
        return 1;
    else
	    return ceil(((double) tamanio_en_bytes)/((double)BLOCK_SIZE));
}

void compactar_y_agregar_fcb(t_fcb* fcb_a_truncar) {

    void* bloquesEnMemoriaCompactado = malloc(BLOCK_SIZE * BLOCK_COUNT);
    if (bloquesEnMemoriaCompactado == NULL) {
        log_error(io_log_debug, "Error al asignar memoria para compactación");
        exit(EXIT_FAILURE);
    }
    
    memset(bloquesEnMemoriaCompactado, 0, BLOCK_SIZE * BLOCK_COUNT);
    
    void* bloque_destino = bloquesEnMemoriaCompactado;
    
    pthread_mutex_lock(&mutex_lista_fat);
    
    t_list_iterator* iter = list_iterator_create(lista_fat);
    int contador_bloques = 0;
    
    pthread_mutex_lock(&mutex_bloquesEnMemoria);
    while (list_iterator_has_next(iter)) {
        t_fcb* fcb = (t_fcb*)list_iterator_next(iter);
        
        if (strcmp(fcb->nombre, fcb_a_truncar->nombre) != 0) {
            // RECORRO LOS BLOQUES DE CADA FCB SIN CONTAR EL QUE QUIERO TRUNCAR Y LOS METO EN UN NUEVO VOID*

            // EL NUEVO BLOQUE INICIAL DEL ARCHIVO
            fcb->bloque_inicial = contador_bloques;
            setear_valor_entero_en_fcb(fcb, "BLOQUE_INICIAL", contador_bloques);
            for (int i = 0; i < fcb->tamanio_en_bloques; i++) {

                memcpy(bloque_destino, bloquesEnMemoria + (fcb->bloque_inicial + i) * BLOCK_SIZE, BLOCK_SIZE);
                bloque_destino += BLOCK_SIZE;
                contador_bloques++;
            }
            
            fcb->bloque_inicial = tamanio_en_bloques(bloque_destino - bloquesEnMemoriaCompactado) - fcb->tamanio_en_bloques;
        }
    }
    
    
    list_iterator_destroy(iter);
    
    memset(bitmapChar, 0, (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0));
    setear_bloque_bits(bitmap, 0, contador_bloques);
    imprimir_bitarray(bitmap);
    
    void* destino_final = bloquesEnMemoriaCompactado + (BLOCK_SIZE * ((bloque_destino - bloquesEnMemoriaCompactado) / BLOCK_SIZE));
    memcpy(destino_final, bloquesEnMemoria + (fcb_a_truncar->bloque_inicial) * BLOCK_SIZE, BLOCK_SIZE * fcb_a_truncar->tamanio_en_bloques);

    fcb_a_truncar->bloque_inicial = (destino_final - bloquesEnMemoriaCompactado) / BLOCK_SIZE;
    fcb_a_truncar->tamanio_en_bloques = tamanio_en_bloques(fcb_a_truncar->tamanio);
    
    setear_bloque_bits(bitmap, fcb_a_truncar->bloque_inicial, fcb_a_truncar->tamanio_en_bloques);
    log_trace(io_log_debug,"ARCHIVO A TRUNCAR: %d \t TAMANIO EN BLOQUES: %d",fcb_a_truncar->bloque_inicial, fcb_a_truncar->tamanio_en_bloques);
    imprimir_bitarray(bitmap);
    memcpy(bloquesEnMemoria, bloquesEnMemoriaCompactado, BLOCK_SIZE * BLOCK_COUNT);
    
    pthread_mutex_unlock(&mutex_bloquesEnMemoria);
    pthread_mutex_unlock(&mutex_lista_fat);
    if(bloquesEnMemoriaCompactado != NULL){

        free(bloquesEnMemoriaCompactado);
        bloquesEnMemoriaCompactado = NULL;
    }
    
}

void escribir_data_en_bloque(int indice, void* valor, size_t tamanio) {
    
    pthread_mutex_lock(&mutex_bloquesEnMemoria);
    memcpy(bloquesEnMemoria + (indice * BLOCK_SIZE), valor, tamanio);
    pthread_mutex_unlock(&mutex_bloquesEnMemoria);
}

void* leer_data_de_bloque(int indice, size_t tamanio){
    void* valor_leido = malloc(tamanio);

    pthread_mutex_lock(&mutex_bloquesEnMemoria);
    memcpy(valor_leido, bloquesEnMemoria + (indice * BLOCK_SIZE), tamanio);
    pthread_mutex_unlock(&mutex_bloquesEnMemoria);

    return valor_leido;
}

