#include "../include/conexion_memoria.h"

void atender_entradasalida_memoria() {
    bool control_key = true;

    while (control_key) {
        int cod_op = recibir_operacion(fd_memoria);
        t_buffer* unBuffer = NULL;
        char* respuesta_escritura;
        switch (cod_op) {
            case LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA:
                unBuffer = __recibiendo_super_paquete(fd_memoria);
                if(data_leida != NULL)
                {
                    free(data_leida);
                    data_leida = NULL;
                }

                data_leida = (char*) __recibir_choclo_del_buffer(unBuffer);
                printf("DATO RECIBIDO DE MEMORIA: %s\n", data_leida);
                destruir_buffer(unBuffer);
                
                sem_post(&sem_leyo_data);
                break;
            case ESCRITURA_BLOQUE_ENTRADASALIDA_MEMORIA: 
                unBuffer = __recibiendo_super_paquete(fd_memoria);
                respuesta_escritura = __recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                log_info(io_log_debug, "RESPUESTA ESCRITURA: %s",respuesta_escritura);
                if(respuesta_escritura != NULL){

                    free(respuesta_escritura);
                    respuesta_escritura = NULL;
                }
                break;
            case -1:
                log_error(io_log_debug, "Desconexión de la memoria");
                control_key = false;
                break;
            default:
                log_warning(io_log_debug, "Operación desconocida de la memoria");
                break;
        }
    }
}

void notificar_memoria_escritura(int pid, int dir_fisica, size_t tamanio, void* data) {
    t_paquete * paquete = __crear_super_paquete(ESCRITURA_BLOQUE_ENTRADASALIDA_MEMORIA);

    __cargar_int_al_super_paquete(paquete, pid);
    __cargar_int_al_super_paquete(paquete, dir_fisica);
    __cargar_size_t_al_super_paquete(paquete, tamanio);
    __cargar_int_al_super_paquete(paquete, T_STRING);
    __cargar_choclo_al_super_paquete(paquete, data, tamanio);

    enviar_paquete(paquete, fd_memoria);
    eliminar_paquete(paquete);
}

void notificar_memoria_lectura(int pid, int dir_fisica, size_t tamanio) {
        
    t_paquete * paquete = __crear_super_paquete(LECTURA_BLOQUE_ENTRADASALIDA_MEMORIA);

    __cargar_int_al_super_paquete(paquete, pid);
    __cargar_int_al_super_paquete(paquete, dir_fisica);
    __cargar_size_t_al_super_paquete(paquete, tamanio);
    __cargar_int_al_super_paquete(paquete, T_STRING);

    enviar_paquete(paquete, fd_memoria);
    eliminar_paquete(paquete);
}