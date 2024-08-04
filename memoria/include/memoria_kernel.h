#ifndef MEMORIA_KERNEL_H_
#define MEMORIA_KERNEL_H_

#include "m_gestor.h"

void atender_memoria_kernel();
void iniciar_estructura_para_un_proceso_nuevo(t_buffer* un_Buffer);
void eliminar_proceso_y_liberar_estructuras(t_buffer* unBuffer);
void responder_a_kernel_confirmacion_del_proceso_creado();
void enviar_a_kernel_rpta_de_eliminacion_de_proceso();

#endif /* MEMORIA_KERNEL_H_ */
