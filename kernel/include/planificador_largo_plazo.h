#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include "k_gestor.h"

void plp_planifica();
void plp_planificar_proceso_nuevo(t_pcb* un_pcb);
void plp_planificar_proceso_exit(int pid);
void plp_exit(t_pcb* pcb);

#endif /* PLANIFICADOR_LARGO_PLAZO_H_ */
