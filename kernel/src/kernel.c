#include "../include/kernel.h"
#include "../include/kernel_cpu_interrupt.h"
#include "../include/inicializar_kernel.h"
#include "../include/finalizar_kernel.h"

int main(int argc, char* argv[]) {
    
    char* archivo_config = argv[1];
    IP_MEMORIA = argv[2];
    IP_CPU = argv[3];
    
    inicializar_kernel(archivo_config);

    // EL KERNEL SE INICIA COMO SERVIDOR
    fd_kernel = iniciar_servidor(PUERTO_ESCUCHA, kernel_log_debug, "KERNEL");

    // EL KERNEL SE CONECTA A MEMORIA
    if ((fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA)))
        log_info(kernel_log_debug, "CONEXION CON MEMORIA");

    // EL KERNEL SE CONECTA A CPU DISPATCH
    if ((fd_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH)))
        log_info(kernel_log_debug, "CONEXION CON CPU DISPATCH");

    // EL KERNEL SE CONECTA A CPU INTERRUPT
    if ((fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT)))
        log_info(kernel_log_debug, "CONEXION CON CPU INTERRUPT");

    pthread_attr_t attr;

    // Inicializar atributos del hilo
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // ATENDER LOS MENSAJES DE MEMORIA
    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria, &attr, (void*) atender_kernel_memoria, NULL);
    pthread_detach(hilo_memoria);

    // // ATENDER LOS MENSAJES DE CPU DISPATCH
    pthread_t hilo_cpu_dispatch;
    pthread_create(&hilo_cpu_dispatch, &attr, (void*) atender_kernel_cpu_dispatch, NULL);
    pthread_detach(hilo_cpu_dispatch);

    // // ATENDER LOS MENSAJES DE CPU INTERRUPT
    pthread_t hilo_cpu_interrupt_consola;
    pthread_create(&hilo_cpu_interrupt_consola, &attr, (void*) _gestionar_interrupt_consola, NULL);
    pthread_detach(hilo_cpu_interrupt_consola);

    pthread_t hilo_cpu_interrupt_quantum;
    pthread_create(&hilo_cpu_interrupt_quantum, &attr, (void*) _gestionar_interrupt_quantum, NULL);
    pthread_detach(hilo_cpu_interrupt_quantum);

    pthread_t hilo_cpu_interrupt_prioridad;
    pthread_create(&hilo_cpu_interrupt_prioridad, &attr, (void*) _gestionar_interrupt_prioridad, NULL);
    pthread_detach(hilo_cpu_interrupt_prioridad);

    pthread_t hilo_escuchar_ES;
    pthread_create(&hilo_escuchar_ES, &attr, (void*) escuchar_ES, NULL);
    pthread_detach(hilo_escuchar_ES);


    pthread_attr_destroy(&attr);

    pthread_t hilo_leer_consola;
    pthread_create(&hilo_leer_consola, NULL, (void*) _leer_consola, NULL);
    pthread_join(hilo_leer_consola, NULL);

    finalizar_kernel();
    printf("TODO KERNEL SE FINALIZO CORRECTAMENTE...\n");

    return EXIT_SUCCESS;
}

// #include "../include/kernel.h"
// #include "../include/kernel_cpu_interrupt.h"
// #include "../include/inicializar_kernel.h"
// #include "../include/finalizar_kernel.h"

// int main(int argc, char* argv[]) {
    
//     char* archivo_config = argv[1];
//     inicializar_kernel(archivo_config);

//     // EL KERNEL SE INICIA COMO SERVIDOR
//     fd_kernel = _iniciar_servidor(PUERTO_ESCUCHA, kernel_log_debug, "KERNEL");

//     // EL KERNEL SE CONECTA A MEMORIA
//     if ((fd_memoria = _crear_conexion(IP_MEMORIA, PUERTO_MEMORIA)))
//         log_info(kernel_log_debug, "CONEXION CON MEMORIA EXITOSA");

//     // EL KERNEL SE CONECTA A CPU DISPATCH
//     if ((fd_cpu_dispatch = _crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH)))
//         log_info(kernel_log_debug, "CONEXION CON CPU DISPATCH EXITOSA");

//     // EL KERNEL SE CONECTA A CPU INTERRUPT
//     if ((fd_cpu_interrupt = _crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT)))
//         log_info(kernel_log_debug, "CONEXION CON CPU INTERRUPT EXITOSA");

//     pthread_attr_t attr;

//     // Inicializar atributos del hilo
//     pthread_attr_init(&attr);
//     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

//     // ATENDER LOS MENSAJES DE MEMORIA
//     pthread_t hilo_memoria;
//     pthread_create(&hilo_memoria, &attr, (void*) atender_kernel_memoria, NULL);
//     pthread_detach(hilo_memoria);

//     // // ATENDER LOS MENSAJES DE CPU DISPATCH
//     pthread_t hilo_cpu_dispatch;
//     pthread_create(&hilo_cpu_dispatch, &attr, (void*) atender_kernel_cpu_dispatch, NULL);
//     pthread_detach(hilo_cpu_dispatch);

//     // // ATENDER LOS MENSAJES DE CPU INTERRUPT
//     pthread_t hilo_cpu_interrupt_consola;
//     pthread_create(&hilo_cpu_interrupt_consola, &attr, (void*) _gestionar_interrupt_consola, NULL);
//     pthread_detach(hilo_cpu_interrupt_consola);

//     pthread_t hilo_cpu_interrupt_quantum;
//     pthread_create(&hilo_cpu_interrupt_quantum, &attr, (void*) _gestionar_interrupt_quantum, NULL);
//     pthread_detach(hilo_cpu_interrupt_quantum);

//     pthread_t hilo_cpu_interrupt_prioridad;
//     pthread_create(&hilo_cpu_interrupt_prioridad, &attr, (void*) _gestionar_interrupt_prioridad, NULL);
//     pthread_detach(hilo_cpu_interrupt_prioridad);

//     pthread_t hilo_escuchar_ES;
//     pthread_create(&hilo_escuchar_ES, &attr, (void*) escuchar_ES, NULL);
//     pthread_detach(hilo_escuchar_ES);


//     pthread_attr_destroy(&attr);

//     pthread_t hilo_leer_consola;
//     pthread_create(&hilo_leer_consola, NULL, (void*) _leer_consola, NULL);
//     pthread_join(hilo_leer_consola, NULL);

//     finalizar_kernel();
//     printf("TODO KERNEL SE FINALIZO CORRECTAMENTE...\n");

//     return EXIT_SUCCESS;
// }

void escuchar_ES(){
    while(server_escucha_entrada_salida());
}

int server_escucha_entrada_salida(){
	log_info(kernel_log_debug, "ESCUCHANDO ENTRADA SALIDA");
	int fd_entradasalida = esperar_cliente(fd_kernel, kernel_log_debug, "ENTRADA SALIDA");

    if(fd_entradasalida != -1){

        int copia_socket = fd_entradasalida;

        t_instancia_args * instancia_args = malloc(sizeof(t_instancia_args));
        instancia_args->socket = fd_entradasalida;

        pthread_t hilo_entradasalida;
        pthread_create(&hilo_entradasalida, NULL, (void*) atender_kernel_entradasalida, (void *) instancia_args); 
        pthread_detach(hilo_entradasalida);

        // ENVIAR PETICION DE NOMBRE Y TIPO A E/S
        enviar_peticion_nombre_y_tipo(copia_socket);

        return 1;
    }
	
	return EXIT_SUCCESS;
}

void enviar_peticion_nombre_y_tipo(int copia_socket) {
    t_paquete * paquete = crear_super_paquete(NOMBRE_ENTRADA_SALIDA);
    cargar_string_al_super_paquete(paquete, "PETICION NOMBRE Y TIPO");
    enviar_paquete(paquete, copia_socket);
    eliminar_paquete(paquete);
}