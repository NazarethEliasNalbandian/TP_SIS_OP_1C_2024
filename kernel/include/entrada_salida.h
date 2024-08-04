#ifndef ENTRADA_SALIDA_H_
#define ENTRADA_SALIDA_H_

#include "k_gestor.h"

bool eliminar_instancia_por_nombre(t_list* lista, char* nombre);
bool existe_interfaz(char* nombre);
t_instancia_io* obtener_instancia_por_nombre(char * nombre_interfaz);
t_instancia_io* obtener_instancia_por_socket(int socket);
char* operacion_to_string(operacion_entradasalida operacion);
bool admite_operacion_solicitada(tipo_interfaz interfaz, operacion_entradasalida operacion);
void destruir_interfaz(t_instancia_io* instancia);
void eliminar_interfaz(t_instancia_io* instancia);
void sacar_pcb_siguiente_de_la_cola_de_espera_y_mandar_a_ejecutar_entrada_salida(t_instancia_io* instancia);
void mandar_pcb_a_entrada_salida(t_instancia_io* instancia,t_pcb_con_mochila* pcb_con_mochila);


#endif

