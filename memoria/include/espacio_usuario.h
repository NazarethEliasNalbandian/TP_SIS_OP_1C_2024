#ifndef ESPACIO_USUARIO_H_
#define ESPACIO_USUARIO_H_

#include "m_gestor.h"
#include "servicios_memoria.h"

void escribir_data_en_dir_fisica(int pid, int dir_fisica, void* valor, size_t tamanio);
void escribir_uint32_en_dir_fisica(int pid, int dir_fisica, uint32_t* valor);
void escribir_uint8_en_dir_fisica(int pid, int dir_fisica, uint8_t* valor);
uint32_t leer_uint32_de_dir_fisica(int pid, int dir_fisica);
uint8_t leer_uint8_de_dir_fisica(int pid, int dir_fisica);
char* leer_string_de_dir_fisica(int pid, int dir_fisica, size_t tamanio);
void* leer_data_de_dir_fisica(int pid, int dir_fisica, size_t tamanio);

#endif 
