#ifndef PLANIFICADOR_CORTO_PLAZO_H_
#define PLANIFICADOR_CORTO_PLAZO_H_

#include "k_gestor.h"

void _programar_interrupcion_por_quantum(t_pcb* un_pcb);
void _atender_RR_FIFO();
void pcp_planificar_corto_plazo();

#endif /* PLANIFICADOR_CORTO_PLAZO_H_ */
