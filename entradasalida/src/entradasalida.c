#include "../include/entradasalida.h"
#include "../include/conexion_kernel.h"
#include "../include/conexion_memoria.h"
#include "../include/inicializar_entradasalida.h"
#include "../include/finalizar_entradasalida.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        log_error(io_log_debug, "Uso: %s <nombre_interfaz> <archivo de configuracion>\n", argv[0]);
        return EXIT_FAILURE;
    }

    NOMBRE_INTERFAZ = argv[1];
    char* archivo_config = argv[2];
    IP_MEMORIA = argv[3];
    IP_KERNEL = argv[4];


    inicializar(archivo_config);

    TIPO_INTERFAZ_ENUM = convertirInterfazAEnum(TIPO_INTERFAZ);

    if ((fd_kernel = _crear_conexion(IP_KERNEL, PUERTO_KERNEL))) {
        log_info(io_log_debug, "Conexión con Kernel exitosa");
    }

    if ((fd_memoria = _crear_conexion(IP_MEMORIA, PUERTO_MEMORIA))) {
        log_info(io_log_debug, "Conexión con Memoria exitosa");
    }

    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) atender_entradasalida_kernel, NULL);
    pthread_detach(hilo_kernel);

    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria, NULL, (void*) atender_entradasalida_memoria, NULL);
    pthread_join(hilo_memoria, NULL);

    finalizar_entradasalida();
    printf("%s SE FINALIZO CORRECTAMENTE...\n", NOMBRE_INTERFAZ);
    return EXIT_SUCCESS;
}

