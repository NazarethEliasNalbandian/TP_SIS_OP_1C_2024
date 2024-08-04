#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include "cpu_gestor.h"

void fetch();
void decode();
uint32_t solicitar_uint32_memoria(int dir_logica);
uint8_t solicitar_uint8_memoria(int dir_logica);
char* solicitar_string_memoria(uint32_t dir_logica, int tamanio);
int escribir_valor_memoria_uint8(uint32_t dir_logica, uint8_t valorAEscribir);
int escribir_valor_memoria_uint32(uint32_t dir_logica, uint32_t valorAEscribir);
int escribir_valor_memoria_string(uint32_t dir_logica, char* valorAEscribir);
void execute();

#endif