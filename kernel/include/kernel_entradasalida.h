#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_

#include "k_gestor.h"

void atender_kernel_entradasalida(void* arg);
void asignar_nombre_y_tipo_y_agregar_a_listas(t_buffer* unBuffer, t_instancia_io* nueva_instancia, int fd_entradasalida);
void atender_fin_entradasalida(t_buffer* unBuffer, t_instancia_io* instancia);

#endif /* KERNEL_ENTRADASALIDA_H_ */
