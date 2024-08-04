#ifndef SERVICIOS_KERNEL_H_
#define SERVICIOS_KERNEL_H_

#include "k_gestor.h"

void pausador();
int generar_verificador();
char* algoritmo_to_string(t_algoritmo algoritmo);
void log_blocked_proceso(int pid_process, char* motivo_block);
float nuevo_quantum(t_temporal* timer);
bool debo_asignar_nuevo_quantum(t_temporal* timer);
void asignoQuantumEnVRR(t_pcb * un_pcb);
bool proceso_ejecutara_io(char * instruccion);
void analizarSiLoMandoAReadyOReadyPrioridad(t_pcb* un_pcb);
t_mochila* crear_mochila();

#endif /* SERVICIOS_KERNEL_H_ */
