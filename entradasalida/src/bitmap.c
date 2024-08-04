#include "../include/bitmap.h"

void imprimir_bitarray(t_bitarray* bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    size_t max_bits = bitarray_get_max_bit(bitarray);
    printf("BITMAP: ");
    for (size_t i = 0; i < max_bits; i++) {
        if (bitarray_test_bit(bitarray, i)) {
            printf("1");
        } else {
            printf("0");
        }
        // Para mejor legibilidad, puedes agregar un espacio cada 8 bits (1 byte)
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
    pthread_mutex_unlock(&mutex_bitmap);
}

int encontrar_primer_bit_libre(t_bitarray* bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    size_t max_bits = bitarray_get_max_bit(bitarray);
    for (size_t i = 0; i < max_bits; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            pthread_mutex_unlock(&mutex_bitmap);
            return i;  // Retorna el índice del primer bit 0 encontrado
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);
    return -1;  // No se encontró ningún bit en 0
}

// USAR CON MUTEX
bool hay_N_bloques_libres_contiguos(t_bitarray *bitarray, int N) {

    return ((encontrar_indice_bits_cero_contiguos(bitarray, N)) != -1);
}

void setear_bloque_bits(t_bitarray *bitarray, int indice, int N) {
    pthread_mutex_lock(&mutex_bitmap);

    for (int i = 0; i < N; i++) {
        bitarray_set_bit(bitarray, indice + i);
        actualizar_bitmap_en_memoria();
    }
    pthread_mutex_unlock(&mutex_bitmap);

}

void cleanear_bloque_bits(t_bitarray *bitarray, int indice, int N) {
    pthread_mutex_lock(&mutex_bitmap);
    for (int i = 0; i < N; i++) {
        bitarray_clean_bit(bitarray, indice + i);
        actualizar_bitmap_en_memoria();
    }
    pthread_mutex_unlock(&mutex_bitmap);
}


int encontrar_indice_bits_cero_contiguos(t_bitarray *bitarray, int N) {
    pthread_mutex_lock(&mutex_bitmap);
    size_t max_bit = bitarray_get_max_bit(bitarray);
    int count = 0; // Contador de bits 0 consecutivos
    int indice_inicio = -1; // Índice del inicio del bloque de bits 0

    for (size_t i = 0; i < max_bit; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            // Encontramos un bit 0
            count++;
            if (count == 1) {
                // Guardar el índice del primer bit 0 del bloque
                indice_inicio = i;
            }
            if (count == N) {
                pthread_mutex_unlock(&mutex_bitmap);
                return indice_inicio; // Encontramos N bits 0 consecutivos, devolvemos el índice inicial
            }
        } else {
            // Encontramos un bit 1, reiniciamos el contador y el índice de inicio
            count = 0;
            indice_inicio = -1;
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);

    return -1; // No se encontraron N bits 0 consecutivos
}

int cantidad_bloques_libres(t_bitarray *bitarray) {
    pthread_mutex_lock(&mutex_bitmap);
    int count = 0;
    size_t max_bits = bitarray_get_max_bit(bitarray);

    for (size_t i = 0; i < max_bits; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            count++;
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);

    return count;
}

void limpiar_N_bloque_anteriores(t_bitarray *bitarray, int indice, int N) {
    pthread_mutex_lock(&mutex_bitmap);
    for (int i = 0; i < N; i++) {
        int current_index = indice - i;
        if (current_index >= 0) {
            bitarray_clean_bit(bitarray, current_index);
            actualizar_bitmap_en_memoria();
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);
}

int contar_bits_cero_consecutivos(t_bitarray *bitarray, int indice) {
    pthread_mutex_lock(&mutex_bitmap);
    int count = 0;
    size_t max_bits = bitarray_get_max_bit(bitarray);

    for (size_t i = indice; i < max_bits; i++) {
        if (bitarray_test_bit(bitarray, i)) {
            break;
        }
        count++;
    }
    pthread_mutex_unlock(&mutex_bitmap);
    return count;
}

void actualizar_bitmap_en_memoria() {
    // Se actualizan los bits en bitmapChar desde la estructura de bitmap
    // Se asume que bitmap y bitmapChar están en la misma región de memoria
    memcpy(bitmapChar, bitmap->bitarray, (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0));
}
