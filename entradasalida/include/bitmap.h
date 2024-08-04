#ifndef BITMAP_H_
#define BITMAP_H_

#include "e_gestor.h"

void imprimir_bitarray(t_bitarray* bitarray); 
int encontrar_primer_bit_libre(t_bitarray* bitarray);
bool hay_N_bloques_libres_contiguos(t_bitarray *bitarray, int N);
void setear_bloque_bits(t_bitarray *bitarray, int indice, int N);
void cleanear_bloque_bits(t_bitarray *bitarray, int indice, int N); 
int encontrar_indice_bits_cero_contiguos(t_bitarray *bitarray, int N);
int cantidad_bloques_libres(t_bitarray* bitarray);
void limpiar_N_bloque_anteriores(t_bitarray *bitarray, int indice, int N);
int contar_bits_cero_consecutivos(t_bitarray *bitarray, int indice);
void actualizar_bitmap_en_memoria();

#endif