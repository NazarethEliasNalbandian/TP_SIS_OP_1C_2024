#ifndef KERNEL_CPU_DISPATCH_H_
#define KERNEL_CPU_DISPATCH_H_

#include "k_gestor.h"

void atender_signal(t_pcb* pcb,char* recurso_a_liberar);
void atender_wait(t_pcb* pcb,char* recurso_solicitado);
void _desalojar_proceso(t_pcb* ctxt);
void atender_io_gen_sleep(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int unidades_trabajo);
void atender_io_stdin(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio);
void atender_io_stdout(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio);
void atender_io_fs_create(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo);
void atender_io_fs_delete(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo);
void atender_io_fs_truncate(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, char* nombre_archivo, size_t tamanio);
void atender_io_fs_write(t_pcb* pcb, t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica, size_t tamanio, char* nombre_archivo, int puntero_archivo);
void atender_io_fs_read(t_pcb* pcb,t_mochila* mochila, t_instancia_io* instancia_entradasalida, int direccion_fisica,size_t tamanio,int puntero_archivo, char* nombre_archivo);
void enviar_pcb_entradasalida_io_gen_sleep(int pid,int  unidades_trabajo, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_stdin(int pid, size_t tamanio,  int direccion_fisica, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_stdout(int pid, size_t tamanio, int direccion_fisica, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_fs_create(int pid, char* nombre_archivo, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_fs_delete(int pid, char* nombre_archivo, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_fs_truncate(int pid, char* nombre_archivo,size_t tamanio, int fd_instancia_entradasalida);
void enviar_pcb_entradasalida_io_fs_write(int pid, size_t tamanio, int direccion_fisica, int fd_instancia_entradasalida, char* nombre_archivo, int puntero_archivo);
void enviar_pcb_entradasalida_io_fs_read(int pid, size_t tamanio, int puntero_archivo, int direccion_fisica, int fd_instancia_entradasalida, char* nombre_archivo);
void atender_kernel_cpu_dispatch();

#endif /* KERNEL_CPU_DISPATCH_H_ */
