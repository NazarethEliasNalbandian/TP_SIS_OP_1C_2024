#ifndef FINALIZAR_KERNEL_H_
#define FINALIZAR_KERNEL_H_

#include "k_gestor.h"

void finalizar_kernel();
void _finalizar_logger();
void _finalizar_config();
void _eliminar_interfases();
void _eliminar_pcbs();
void _finalizar_recursos();
void _destruir_conexiones();
void _finalizar_listas();
void _finalizar_semaforos();
void _finalizar_pthread();

#endif /* FINALIZAR_KERNEL_H_ */
