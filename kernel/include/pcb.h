#ifndef PCB_H_
#define PCB_H_

#include "k_gestor.h"

char* estado_to_string(int valor_estado);
char* motivo_to_string(t_motivo_exit motivo_exit);
t_pcb* crear_pcb(char* path);
void cambiar_estado(t_pcb* un_pcb, est_pcb nex_state);
void imprimir_pcb(t_pcb* un_pcb);
void _enviar_pcb_CPU_dispatch(t_pcb* un_pcb);
char* lista_pids_en_ready();
void agrego_datos_pcb(t_pcb* un_pcb);
void destruir_pcb(t_pcb* un_pcb);
void destruir_mochila(t_mochila* mochila);
void destruir_pcb_con_mochila(t_pcb_con_mochila* pcb_con_mochila);
t_pcb* buscar_y_remover_pcb_por_pid(int un_pid);
void avisar_a_memoria_para_liberar_estructuras(t_pcb* un_pcb);
void transferir_from_actual_to_siguiente(t_pcb* pcb, t_list* lista_siguiente, pthread_mutex_t mutex_siguiente, est_pcb estado_siguiente);
void asignar_recurso_liberado_pcb(t_recurso* un_recurso);
void liberar_recursos_pcb(t_pcb* pcb);
void agregar_pcb_lista(t_pcb* pcb, t_list* lista_estado, pthread_mutex_t mutex_lista);
t_pcb* buscar_pcb_por_pid_en(int un_pid, t_list* lista_estado);
bool esta_pcb_en_una_lista_especifica(t_list* una_lista, t_pcb* un_pcb);
bool buscar_pcb(t_pcb* void_pcb);
t_pcb* obtener_proceso_desalojado(t_contexto* ctxt);
t_mochila* copiar_mochila(t_mochila* original);
t_pcb* duplicar_pcb(t_pcb* original);
void* duplicar_recurso(void* recurso_original);

#endif /* PCB_H_ */