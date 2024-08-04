#ifndef CPU_KERNEL_DISPATCH_H_
#define CPU_KERNEL_DISPATCH_H_

#include "cpu_gestor.h"

uint8_t *detectar_registro(char *RX);
uint32_t *detectar_registroE(char *ERX);
void destruir_contexto();
bool check_interrupt();
void ciclo_de_instruccion();
void atender_cpu_kernel_dispatch();
void atender_proceso_kernel();
void atender_cpu_kernel_dispatch();
void enviar_contexto_a_kernel_con_desalojo(op_code tipo_desalojo);

#endif 