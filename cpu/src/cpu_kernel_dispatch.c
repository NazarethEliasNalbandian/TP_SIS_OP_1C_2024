#include "../include/cpu_kernel_dispatch.h"
#include "../include/ciclo_instruccion.h"
#include "../include/servicios_cpu.h"

uint8_t *detectar_registro(char *RX)
{
    if (strcmp(RX, "AX") == 0)
    {
        return &(un_contexto->r_cpu->AX);
    }
    else if (strcmp(RX, "BX") == 0)
    {
        return &(un_contexto->r_cpu->BX);
    }
    else if (strcmp(RX, "CX") == 0)
    {
        return &(un_contexto->r_cpu->CX);
    }
    else if (strcmp(RX, "DX") == 0)
    {
        return &(un_contexto->r_cpu->DX);
    }
    else
    {
        return &(un_contexto->r_cpu->AX);
    }
}
uint32_t *detectar_registroE(char *ERX)
{
    if (strcmp(ERX, "EAX") == 0)
    {
        return &(un_contexto->r_cpu->EAX);
    }
    else if (strcmp(ERX, "EBX") == 0)
    {
        return &(un_contexto->r_cpu->EBX);
    }
    else if (strcmp(ERX, "ECX") == 0)
    {
        return &(un_contexto->r_cpu->ECX);
    }
    else if (strcmp(ERX, "EDX") == 0)
    {
        return &(un_contexto->r_cpu->EDX);
    }
        else if (strcmp(ERX, "PC") == 0)
    {
        return &(un_contexto->r_cpu->PC);
    }
    else if (strcmp(ERX, "SI") == 0)
    {
        return &(un_contexto->r_cpu->SI);
    }
    else if (strcmp(ERX, "DI") == 0)
    {
        return &(un_contexto->r_cpu->DI);
    }
    else
    {
        return &(un_contexto->r_cpu->EAX);
    }
}

void destruir_contexto(){

    if(un_contexto->r_cpu != NULL){

        free(un_contexto->r_cpu);
        un_contexto->r_cpu = NULL;
    }

    if(un_contexto != NULL){
        free(un_contexto);
        un_contexto = NULL;
    }

    // if(interrupt_pid!= NULL){
	// 	// free(interrupt_pid);
	// 	// free(interrupt_verificador);
	// 	// free(interrupt_motivo);
	// 	interrupt_pid = NULL;
	// 	interrupt_verificador = NULL;
	// 	interrupt_motivo = NULL;
	// }

	pthread_mutex_lock(&mutex_interruptFlag);
	interrupt_flag = false;
	pthread_mutex_unlock(&mutex_interruptFlag);
	desalojar = false;
}

bool check_interrupt()
{
    pthread_mutex_lock(&mutex_interruptFlag);
    bool interrupt = interrupt_flag;
    pthread_mutex_unlock(&mutex_interruptFlag);
    if(interrupt)
    {
        return 1;
    }
    return 0;
}

void ciclo_de_instruccion()
{
    fetch();
	sem_wait(&sem_fetch);
	decode();
	sem_wait(&sem_decode);
	execute();
}

void atender_proceso_kernel(t_buffer* unBuffer){

    // pthread_mutex_lock(&mutex_manejo_contexto);

    pthread_mutex_lock(&mutex_manejo_contexto);
    recibir_contexto(unBuffer, un_contexto);
    pthread_mutex_unlock(&mutex_manejo_contexto);
                
    log_info(cpu_log_debug, "RECIBI EL PID %d PARA EJECUTAR", un_contexto->pID);

    pthread_mutex_lock(&mutex_tipo_desalojo);
    
    
    while(1){
        //pthread_mutex_lock(&mutex_manejo_contexto);
        ciclo_de_instruccion();
        sem_post(&sem_inst);
        //pthread_mutex_unlock(&mutex_manejo_contexto);

        // pthread_mutex_lock(&mutex_ejecute_wait_y_no_hay_instancias);
        if(ejecute_wait_y_no_hay_instancias){
            break;
        }

        if(hay_io_gen){
            break;
        }

        // pthread_mutex_unlock(&mutex_ejecute_wait_y_no_hay_instancias);

        if(hay_io(nombre_instruccion_enum)){
            break;
        }
        if(desalojar){
            break;
        }
        
        pthread_mutex_lock(&mutex_interruptFlag);
		bool bool_interrupt = interrupt_flag;
		pthread_mutex_unlock(&mutex_interruptFlag);

		//Controlar si hay interrupciones provenientes de kernel
		if(bool_interrupt){
			break;
		}
    }
    t_paquete* un_paquete;
    log_info(cpu_log_debug, "CPU dejo de procesar, se interrumpio el ciclo");
    pthread_mutex_lock(&mutex_manejo_contexto);
    pthread_mutex_lock(&mutex_interruptFlag);
    if(!flag_exit && interrupt_flag){
        log_info(cpu_log_debug, "ENTRE A INTERRUPT_FLAG");
		un_paquete = crear_super_paquete(ATENDER_DESALOJO_PROCESO_CPU);
        agregar_contexto_a_paquete(un_paquete);
        cargar_string_al_super_paquete(un_paquete, interrupt_motivo);
        enviar_paquete(un_paquete, fd_kernel_dispatch);
        eliminar_paquete(un_paquete);
	}
    if(desalojar){
        // ENTRA ACA EN CASO DE ERRORES O EXIT
        log_info(cpu_log_debug, "ENTRE A DESALOJAR_FLAG");
        un_paquete = crear_super_paquete(tipo_desalojo);
        agregar_contexto_a_paquete(un_paquete);
        enviar_paquete(un_paquete, fd_kernel_dispatch);
        eliminar_paquete(un_paquete);
	}
	pthread_mutex_unlock(&mutex_interruptFlag);
    destruir_buffer(unBuffer);  
	log_info(cpu_log_obligatorio, "Proceso_desalojado <PID:%d>",un_contexto->pID);
	// destruir_contexto();
	// log_info(cpu_log_debug, "Todo el contexto se elimino correctamente");
    ejecute_wait_y_no_hay_instancias = false;
    desalojar = false;
    interrupt_flag = false;
    flag_exit = false;
    if(instruccion_split != NULL){

        string_array_destroy(instruccion_split);
        instruccion_split = NULL;
    }

	pthread_mutex_unlock(&mutex_manejo_contexto);
    pthread_mutex_unlock(&mutex_tipo_desalojo);
    //pthread_mutex_unlock(&mutex_manejo_contexto);
}

void atender_cpu_kernel_dispatch()
{
    bool control_key = 1;
    while (control_key){
        int cod_op = recibir_operacion(fd_kernel_dispatch);
        t_buffer* unBuffer = NULL;

        switch (cod_op){
            case MENSAJE:
                recibir_mensaje(fd_kernel_dispatch, cpu_log_debug);
                break;
            case EJECUTAR_PROCESO_KC:
                unBuffer = recibir_paquete(fd_kernel_dispatch);

                ejecutar_en_un_hilo_nuevo_detach((void*)atender_proceso_kernel, (void*)  unBuffer);
                break;
            case ATENDER_RTA_KERNEL:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                rta_kernel = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);

                log_info(cpu_log_debug, "RECIBI RESPUESTA DE KERNEL %s", rta_kernel);
                
                if(strcmp(rta_kernel, "-1") == 0){
                    log_warning(cpu_log_obligatorio, "PID: <%d> - Voy a tener que desalojar %d",un_contexto->pID, desalojar);
                    // pthread_mutex_lock(&mutex_ejecute_wait_y_no_hay_instancias);

                    ejecute_wait_y_no_hay_instancias = true;
                    // pthread_mutex_unlock(&mutex_ejecute_wait_y_no_hay_instancias);
                }
                
                sem_post(&sem_rta_kernel);
                if(rta_kernel != NULL)
                {
                    free(rta_kernel);
                    rta_kernel = NULL;
                }
                break;
            case -1:
                log_error(cpu_log_debug, "Desconexión de Kernel Dispatch");
                control_key = 0;
                break;
            default:
                log_warning(cpu_log_debug, "Operacion desconocida de Kernel Dispatch.");
                break;
        }
    }
    log_warning(cpu_log_debug, "Saliendo del hilo de CPU_DISPATCH - KERNEL");
}