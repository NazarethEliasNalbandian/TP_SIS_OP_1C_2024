#ifndef INICIALIZAR_ENTRADASALIDA_H_
#define INICIALIZAR_ENTRADASALIDA_H_

#include "../include/e_gestor.h"

void inicializar(char* archivo_config);

void inicializar_logs();
void inicializar_configs(char* archivo_config);
void inicializar_semaforos();
void inicializar_estructuras_filesystem();
void inicializar_archivo_bloques();
void inicializar_archivo_bitmap();
void inicializar_pthreads();

#endif