#ifndef CONEXION_KERNEL_H_
#define CONEXION_KERNEL_H_

#include "e_gestor.h"

void atender_entradasalida_kernel();
void procesar_sleep(t_buffer* buffer);
void procesar_stdin(t_buffer* buffer);
void procesar_stdout(t_buffer* buffer);
void procesar_fs_create(t_buffer* unBuffer);
void procesar_fs_delete(t_buffer* unBuffer);
void procesar_fs_truncate(t_buffer* unBuffer);
void procesar_fs_write(t_buffer* unBuffer);
void procesar_fs_read(t_buffer* unBuffer);
void enviar_nombre_a_kernel(t_buffer* unBuffer);
void enviar_fin_entradasalida_a_kernel(int pid);
void enviar_contexto_a_kernel_con_desalojo(int pid);
void notificar_memoria_escritura(int pid, int dir_fisica, size_t tamanio, void* data);
void notificar_memoria_lectura(int pid, int dir_fisica, size_t tamanio); 
void enviar_peticion_tamanio_pagina();

#endif /* CONEXION_KERNEL_H_ */
