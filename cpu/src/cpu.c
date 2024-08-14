#include "../include/cpu.h"

int main(int argc, char* argv[]) {

    char* archivo_config = argv[1];
    IP_MEMORIA = argv[2];

    inicializar_cpu(archivo_config);
    // cpu servidor espera conexion kernel d e i
    fd_cpu_dispatch =iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_log_debug, "CPU DISPATCH");
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_log_debug, "CPU INTERRUPT");
    //  cpu cliente se conecta a memoria y envia handshake
    fd_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    log_info(cpu_log_debug,"MEMORIA");
    //  cpu servidor acepta kernel d e i
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_log_debug, "KERNEL DISPATCH");
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_log_debug, "KERNEL INTERRUPT");

    //  cpu servidor se comunica kernel d e i
    pthread_t hilo_kernel_dispatch, hilo_kernel_interrupt, hilo_memoria;
    pthread_create(&hilo_kernel_dispatch,NULL,(void*)atender_cpu_kernel_dispatch,NULL);
    pthread_detach(hilo_kernel_dispatch);
    
    pthread_create(&hilo_kernel_interrupt,NULL,(void*)atender_cpu_kernel_interrupt,NULL);
    pthread_detach(hilo_kernel_interrupt);
    //  cpu cliente se comunica con memoria
    pthread_create(&hilo_memoria,NULL,(void*)atender_cpu_memoria,NULL);
    pthread_join(hilo_memoria,NULL);

    finalizar_cpu(); //crear despues
    printf("TODO CPU SE FINALIZO CORRECTAMENTE...\n");

    return EXIT_SUCCESS;
}