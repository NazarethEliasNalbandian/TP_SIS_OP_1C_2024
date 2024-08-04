#ifndef BLOQUES_H_
#define BLOQUES_H_

#include "e_gestor.h"

int tamanio_en_bloques(int tamanio_en_bytes);
void compactar_y_agregar_fcb(t_fcb* fcb_a_truncar);
void escribir_data_en_bloque(int indice, void* valor, size_t tamanio);
void* leer_data_de_bloque(int indice, size_t tamanio);

#endif