#include "../include/incializar_cpu.h"
#include "../include/dic_operaciones.h"

void iniciar_semaforos(){
    sem_init(&sem_fetch, 0, 0);
	sem_init(&sem_decode, 0, 0);
	sem_init(&sem_execute, 0, 0);
	sem_init(&sem_resize, 0, 0);
	sem_init(&sem_val_leido, 0, 0);
    sem_init(&sem_val_escrito, 0, 0);
	sem_init(&sem_sol_marco, 0, 0);
    sem_init(&sem_rta_kernel, 0, 0);
    sem_init(&sem_inst,0,1);
    sem_init(&sem_nro_marco,0,1);

	pthread_mutex_init(&mutex_interruptFlag, NULL);
	pthread_mutex_init(&mutex_manejo_contexto, NULL);
    pthread_mutex_init(&mutex_ejecute_wait_y_no_hay_instancias, NULL);
    pthread_mutex_init(&mutex_tipo_desalojo, NULL);

    pthread_mutex_init(&mutex_interrupt_pid, NULL);
    pthread_mutex_init(&mutex_interrupt_verificador, NULL);
    pthread_mutex_init(&mutex_interrupt_motivo, NULL);
    pthread_mutex_init(&mutex_desalojar, NULL);
    pthread_mutex_init(&mutex_fallo_algoritmo, NULL);
    pthread_mutex_init(&mutex_instruccion, NULL);
    pthread_mutex_init(&mutex_rta_m_resize, NULL);
    pthread_mutex_init(&mutex_rta_kernel, NULL);
    pthread_mutex_init(&mutex_flag_exit, NULL);
    pthread_mutex_init(&mutex_valor_leido_string, NULL);
    pthread_mutex_init(&mutex_valor_leido_uint32, NULL);
    pthread_mutex_init(&mutex_valor_leido_uint8, NULL);
    pthread_mutex_init(&mutex_valor_escrito, NULL);
    pthread_mutex_init(&mutex_op_autorizada, NULL);
    pthread_mutex_init(&mutex_instruccion_split, NULL);
    pthread_mutex_init(&mutex_TLB, NULL);
    pthread_mutex_init(&mutex_TLB_indices, NULL);
    pthread_mutex_init(&mutex_nro_marco, NULL);
    pthread_mutex_init(&mutex_nombre_instruccion_enum, NULL);
}

void inicializar_cpu(char* archivo_config){

    cpu_log_debug = log_create("cpu.log","CPU_LOG",1,LOG_LEVEL_TRACE);

    cpu_log_obligatorio = log_create("cpu_ob.log","CPU_LOG_OB",1,LOG_LEVEL_INFO);

    cpu_config = config_create(archivo_config);

    if(cpu_config == NULL){
		log_error(cpu_log_debug, "No se encontro el path del config\n");
		config_destroy(cpu_config);
		log_destroy(cpu_log_debug);
		exit(2);
	}

    // IP_MEMORIA               = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA           = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH  = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT");
    CANT_ENTRADAS_TLB        = config_get_int_value(cpu_config,"CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB            = config_get_string_value(cpu_config,"ALGORITMO_TLB");

    interrupt_flag=0;
    desalojar=0;
    fallo_algoritmo=0;
    ejecute_wait_y_no_hay_instancias = false;
    flag_exit = false;
    hay_io_gen =false;

    TLB = list_create();
    TLB_indices = list_create();

    un_contexto = malloc(sizeof(t_contexto));
    un_contexto->r_cpu = malloc(sizeof(t_registrosCPU));

    diccionario_operaciones();

    interrupt_motivo = NULL;
    valor_leido_string = NULL;

    iniciar_semaforos();

}