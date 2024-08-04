#ifndef FCB_H_
#define FCB_H_

#include "e_gestor.h"

void inicializar_fcb(char* nombre_archivo);
void crear_fcb(char* nombre_archivo);
t_fcb* obtener_fcb(char* nombre_archivo);
void setear_valor_entero_en_fcb(t_fcb* una_fcb, char* clave, int valor);
void setear_valor_string_en_fcb(t_fcb* una_fcb, char* clave, char* valor);
void destruir_fcb(t_fcb* fcb);
void destruir_listas_fcbs();
void eliminar_fcb_de_fat(char* nombre_archivo);

#endif