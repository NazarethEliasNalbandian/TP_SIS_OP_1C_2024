#ifndef KERNEL_CPU_INTERRUPT_H_
#define KERNEL_CPU_INTERRUPT_H_

#include "k_gestor.h"

void _gestionar_interrupt_consola();
void _gestionar_interrupt_quantum();
void _gestionar_interrupt_prioridad();
void _desalojar_proceso(t_pcb* un_pcb);
void _reubicar_pcb_de_execute_a_ready(t_pcb* un_pcb, char* motivo_desalojo);
void _atender_motivo_desalojo(char* motivo_desalojo, t_pcb* un_pcb);


#endif /* KERNEL_CPU_INTERRUPT_H_ */
