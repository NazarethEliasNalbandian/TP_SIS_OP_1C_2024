#ifndef CPU_KERNEL_INTERRUPT_H_
#define CPU_KERNEL_INTERRUPT_H_

#include "cpu_gestor.h"

void atender_cpu_kernel_interrupt();
void manejar_interrupcion(t_buffer * unBuffer);

#endif