#ifndef INICIALIZAR_KERNEL_H_
#define INICIALIZAR_KERNEL_H_

#include "k_gestor.h"

void inicializar_kernel(char* archivo_config);
void inicializar_logs();
void inicializar_configs(char* archivo_config);
void iniciar_semaforos();
void iniciar_listas();
void iniciar_pthread();
void _iniciar_recursos();

#endif /* INICIALIZAR_KERNEL_H_ */
