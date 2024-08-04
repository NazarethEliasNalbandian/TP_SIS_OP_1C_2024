#include "../include/conexion_kernel.h"
#include "../include/conexion_memoria.h"
#include "../include/bitmap.h"
#include "../include/fcb.h"
#include "../include/bloques.h"

void atender_entradasalida_kernel() {
    bool control_key = true;

    while (control_key) {
        int cod_op = recibir_operacion(fd_kernel);
        sem_wait(&sem_proceso_en_io);
        t_buffer* unBuffer = NULL;
        switch (cod_op) {
            case NOMBRE_ENTRADA_SALIDA:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                enviar_nombre_a_kernel(unBuffer);
                break;
            case PCB_SLEEP:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_sleep(unBuffer);
                break;
            case PCB_STDIN:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_stdin(unBuffer);
                break;
            case PCB_STDOUT:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_stdout(unBuffer);
                break;
            case PCB_FS_CREATE:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_fs_create(unBuffer);
                break;
            case PCB_FS_DELETE:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_fs_delete(unBuffer);
                break;
            case PCB_FS_TRUNCATE:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_fs_truncate(unBuffer);
                break;
            case PCB_FS_WRITE:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_fs_write(unBuffer);
                break;
            case PCB_FS_READ:
                unBuffer = __recibiendo_super_paquete(fd_kernel);
                procesar_fs_read(unBuffer);
                break;
            case -1:
                log_error(io_log_debug, "Desconexión del kernel");
                control_key = false;
                break;
            default:
                log_warning(io_log_debug, "Operación desconocida del kernel");
                break;
        }
        sem_post(&sem_proceso_en_io);
    }
}

void procesar_sleep(t_buffer* buffer) {
    int pid = __recibir_int_del_buffer(buffer);
    int tiempo_a_dormir = __recibir_int_del_buffer(buffer);
    destruir_buffer(buffer);

    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_GEN_SLEEP", pid);
    usleep(tiempo_a_dormir * 1000);
    enviar_fin_entradasalida_a_kernel(pid);
}

void procesar_stdin(t_buffer* buffer) {
    int pid = __recibir_int_del_buffer(buffer);
    size_t tamanio = extraer_size_t_del_buffer(buffer);
    int direccion_fisica = __recibir_int_del_buffer(buffer);

    log_info(io_log_debug, "TAMANIO: %ld", tamanio);

    destruir_buffer(buffer);

    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_STDIN_READ", pid);

    char* data = malloc(tamanio + 1);

    printf("Ingrese el valor para STDIN: ");
    if (fgets(data, tamanio + 1, stdin) == NULL) {
        log_error(io_log_debug, "ERROR AL LEER");
        if(data != NULL){
            free(data);
            data = NULL;
        }

        return;
    }

    // Remueve el carácter de nueva línea si está presente
    size_t len = strlen(data);
    if (len > 0 && data[len - 1] == '\n') {
        data[len - 1] = '\0';
    } else {
        limpiar_buffer_entrada();  // Limpia el buffer de entrada si hay más caracteres
    }

    notificar_memoria_escritura(pid, direccion_fisica, tamanio, data);

    if(data != NULL){

        free(data);
        data = NULL;

    }
    enviar_fin_entradasalida_a_kernel(pid);
}

void procesar_stdout(t_buffer* buffer) {
    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
    int pid = __recibir_int_del_buffer(buffer);
    size_t tamanio = extraer_size_t_del_buffer(buffer);
    int direccion_fisica = __recibir_int_del_buffer(buffer);

    destruir_buffer(buffer);

    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_STDOUT_WRITE", pid);
 
    notificar_memoria_lectura(pid, direccion_fisica, tamanio);
    sem_wait(&sem_leyo_data);
    
    enviar_fin_entradasalida_a_kernel(pid);
}

void procesar_fs_create(t_buffer* unBuffer){

    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
    int pid = __recibir_int_del_buffer(unBuffer);
    char* nombre_archivo = __recibir_string_del_buffer(unBuffer);
    destruir_buffer(unBuffer);
    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_FS_CREATE", pid);

    // CHEQUEO Q HAYA UN BLOQUE LIBRE EN EL BITMAP
    
    int indice = encontrar_primer_bit_libre(bitmap);
    
    if(indice != -1){
        // HAY N BLOQUES LIBRES
        // CARGO EL ARCHIVO DE METADATA
        inicializar_fcb(nombre_archivo);
        pthread_mutex_lock(&mutex_lista_fat);
        t_fcb* una_fcb = obtener_fcb(nombre_archivo);
        setear_valor_entero_en_fcb(una_fcb, "BLOQUE_INICIAL", indice);
        pthread_mutex_unlock(&mutex_lista_fat);
        una_fcb->bloque_inicial = indice;

        log_trace(io_log_debug,"INDICE ASOCIADO AL ARCHIVO %s: %d", una_fcb->nombre, una_fcb->bloque_inicial);

        // ACTUALIZO EL BITMAP
        setear_bloque_bits(bitmap, indice, 1);
        imprimir_bitarray(bitmap);

        log_info(io_log_obligatorio, "PID: <%d> - Crear Archivo: <%s>", pid, nombre_archivo);
        enviar_fin_entradasalida_a_kernel(pid);
    }
    else{
        log_error(io_log_debug, "No hay bloques libres");
        enviar_contexto_a_kernel_con_desalojo(pid);
    }  
    if(nombre_archivo != NULL){

        free(nombre_archivo);
        nombre_archivo = NULL;
    }  
}

void procesar_fs_delete(t_buffer* unBuffer){

    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
    int pid = __recibir_int_del_buffer(unBuffer);
    char* nombre_archivo = __recibir_string_del_buffer(unBuffer);
    destruir_buffer(unBuffer);
     log_info(io_log_obligatorio, "PID: %d - Operacion: IO_FS_DELETE", pid);

    // ME FIJO EN LA TABLA Y BUSCO EL FCB ASOCIADO A ESE ARCHIVO
    pthread_mutex_lock(&mutex_lista_fat);
    t_fcb* una_fcb = obtener_fcb(nombre_archivo);
    pthread_mutex_unlock(&mutex_lista_fat);

    if(una_fcb == NULL)
    {
        pthread_mutex_lock(&mutex_lista_fat);
        crear_fcb(nombre_archivo);
        una_fcb = obtener_fcb(nombre_archivo);
        pthread_mutex_unlock(&mutex_lista_fat);
    }

    // ME FIJO EL BLOQUE INICIAL Y LO GUARDO OBTENIENDO EL INDICE
    int indice = una_fcb->bloque_inicial;

    // ME FIJO EL TAMANIO EN BLOQUES Y ESE VA A SER LA CANTIDAD DE BLOQUES A ELIMINAR A PARTIR DEL BLOQUE
    int cantidad_bloques = una_fcb->tamanio_en_bloques;

    // A ESOS BLOQUES LOS VOY A PONER EN 1 EN EL BITMAP
    cleanear_bloque_bits(bitmap, indice, cantidad_bloques);
    imprimir_bitarray(bitmap);

    pthread_mutex_lock(&mutex_lista_fat);
    eliminar_fcb_de_fat(nombre_archivo);
    pthread_mutex_unlock(&mutex_lista_fat);

	log_info(io_log_obligatorio, "PID: <%d> - Eliminar Archivo: <%s>", pid, nombre_archivo);
    enviar_fin_entradasalida_a_kernel(pid);
    if(nombre_archivo != NULL){
        free(nombre_archivo);
        nombre_archivo = NULL;
    }  
}

void procesar_fs_truncate(t_buffer* unBuffer){  

    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);
    int pid = __recibir_int_del_buffer(unBuffer);
    char* nombre_archivo = __recibir_string_del_buffer(unBuffer);
    int nuevo_tamanio = __recibir_int_del_buffer(unBuffer);
    destruir_buffer(unBuffer);
    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_FS_TRUNCATE", pid);

    // ME FIJO EN LA TABLA Y BUSCO EL FCB ASOCIADO A ESE ARCHIVO
    pthread_mutex_lock(&mutex_lista_fat);
    t_fcb* una_fcb = obtener_fcb(nombre_archivo);
    pthread_mutex_unlock(&mutex_lista_fat);

    if(una_fcb == NULL)
    {
        pthread_mutex_lock(&mutex_lista_fat);
        crear_fcb(nombre_archivo);
        una_fcb = obtener_fcb(nombre_archivo);
        pthread_mutex_unlock(&mutex_lista_fat);
    }

    // CALCULO LA CANTIDAD DE BLOQUES LIBRES CON LA CANTIDAD Q NECESITO
    // SI NO HAY SUFICIENTES BLOQUES, LANZO ERROR

    // UN BLOQUE QUE TIENE BLOQUE SIZE 

    // NUEVO TAMANIO 80
    // BLOQUE 16
    // TAMANIO ACTUAL 0
    // CANT_BLOQUES_ACTUALES 1
    // CANT_BLOQUES FINALES = ceil(80/16) = 5 
    // CANT BLOQUES A AGREGAR = 5-1 = 4 

    int indice = una_fcb->bloque_inicial;

    int cantidad_bloques_finales = tamanio_en_bloques(nuevo_tamanio - (int) una_fcb->tamanio);

    int cantidad_bloques_a_agregar = cantidad_bloques_finales - una_fcb->tamanio_en_bloques;

    int cantidad_bloques_disponibles = cantidad_bloques_libres(bitmap);

    int cantidad_bits_ceros_consecutivos = 0;

    printf("INDICE: %d\n", indice);
    printf("CANT BLOQUES FINALES : %d\n", cantidad_bloques_finales);
    printf("CANT BLOQUES DISPONIBLES: %d\n", cantidad_bloques_disponibles);
    printf("CANT BLOQUES ACTUALES: %d\n", una_fcb->tamanio_en_bloques);



    if (cantidad_bloques_disponibles < cantidad_bloques_a_agregar){
        imprimir_bitarray(bitmap);

    }
    else{
        // ME FIJO SI QUIERO AUMENTAR O DISMINUIR EL TAMAÑO
        if(cantidad_bloques_a_agregar == 0){
            una_fcb->tamanio_en_bloques = tamanio_en_bloques(nuevo_tamanio);
            pthread_mutex_lock(&mutex_lista_fat);
            setear_valor_entero_en_fcb(una_fcb, "TAMANIO_ARCHIVO", nuevo_tamanio);
            pthread_mutex_unlock(&mutex_lista_fat);
            una_fcb->tamanio = nuevo_tamanio;
            log_info(io_log_obligatorio,"PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%ld>", pid, una_fcb->nombre, una_fcb->tamanio);
        }
        else if(cantidad_bloques_a_agregar < 0){
            // SI QUIERO DISMINUIR:
            // CALCULO LA NUEVA CANTIDAD DE BLOQUES Y LA RESTO CON LA ORIGINAL
            // LUEGO, PONGO EN 0 EN EL BITMAP ESOS BLOQUES Q YA NO USO MÁS
            limpiar_N_bloque_anteriores(bitmap, indice + una_fcb->tamanio_en_bloques, (-1) * cantidad_bloques_a_agregar);
            pthread_mutex_lock(&mutex_lista_fat);
            setear_valor_entero_en_fcb(una_fcb, "TAMANIO_ARCHIVO", nuevo_tamanio);
            pthread_mutex_unlock(&mutex_lista_fat);
            una_fcb->tamanio_en_bloques = tamanio_en_bloques(nuevo_tamanio);
            una_fcb->tamanio = nuevo_tamanio;
            log_info(io_log_obligatorio,"PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%ld>", pid, una_fcb->nombre, una_fcb->tamanio);
            imprimir_bitarray(bitmap);
        }
        else{
            
            // SI QUIERO AUMENTAR EL TAMAÑO:
            // ME FIJO SI HAY SUFICIENTES BLOQUES LIBRES SIGUIENTES PARA CAMBIAR EL TAMANIO
            // SI LOS HAY, MODIFICO EL BITMAP OCUPANDO ESOS BLOQUES Y ACTUALIZO EL FCB ASOCIADO

            cantidad_bits_ceros_consecutivos = contar_bits_cero_consecutivos(bitmap, indice + una_fcb->tamanio_en_bloques);

            if(cantidad_bits_ceros_consecutivos >= cantidad_bloques_a_agregar){
                // PONE EN 1 LOS N BITS CONSECUTIVOS A UN INDICE DE UN BITMAP
                setear_bloque_bits(bitmap, indice + una_fcb->tamanio_en_bloques, cantidad_bloques_a_agregar);

                una_fcb->tamanio_en_bloques = tamanio_en_bloques(nuevo_tamanio);
                pthread_mutex_lock(&mutex_lista_fat);
                setear_valor_entero_en_fcb(una_fcb, "TAMANIO_ARCHIVO", nuevo_tamanio);
                pthread_mutex_unlock(&mutex_lista_fat);
                una_fcb->tamanio = nuevo_tamanio;


                log_info(io_log_obligatorio,"PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%ld>", pid, una_fcb->nombre, una_fcb->tamanio);
                imprimir_bitarray(bitmap);
            }
            else if (cantidad_bits_ceros_consecutivos < cantidad_bloques_a_agregar){
                // SI NO LOS HAY, REALIZO LA COMPACTACION

                log_info(io_log_obligatorio, "PID: <%d> - Inicio Compactación.", pid);

                // CREO UN NUEVO VOID*, LEO CADA FCB DE LA LISTA FAT SIN CONTAR EL QUE QUIERO CAMBIAR DE TAMAÑO
                // COPIO LA INFORMACIÓN DE LOS BLOQUES DE UN FCB EN EL NUEVO VOID* CAMBIANDO EL BLOQUE INCIAL Y HACIENDO QUE ESTÉN TODOS CONTIGUOS
                // A MEDIDA QUE COPIO LOS BLOQUES, ACTUALIZO EL BITMAP Y EL BLOQUE INICIAL DE CADA FCB
                // UNA VEZ TERMINADO TODOS LOS FCB, AGREGO EL FCB A TRUNCAR AL FINAL
                una_fcb->tamanio = nuevo_tamanio;
                compactar_y_agregar_fcb(una_fcb);
                pthread_mutex_lock(&mutex_lista_fat);
                setear_valor_entero_en_fcb(una_fcb, "TAMANIO_ARCHIVO", nuevo_tamanio);
                setear_valor_entero_en_fcb(una_fcb, "BLOQUE_INICIAL", una_fcb->bloque_inicial);
                pthread_mutex_unlock(&mutex_lista_fat);

                usleep(RETRASO_COMPACTACION * 1000);

                log_info(io_log_obligatorio, "PID: <%d> - Fin Compactación.", pid);
                log_info(io_log_obligatorio,"PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%ld>", pid, una_fcb->nombre, una_fcb->tamanio);
            }
            
        }
    }

    enviar_fin_entradasalida_a_kernel(pid);
    if(nombre_archivo != NULL){
        free(nombre_archivo);
        nombre_archivo = NULL;
    }  

}

void procesar_fs_write(t_buffer* unBuffer){
    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

    int pid = __recibir_int_del_buffer(unBuffer);
    char* nombre_archivo = __recibir_string_del_buffer(unBuffer);
    size_t tamanio = extraer_size_t_del_buffer(unBuffer);
    int puntero_archivo = __recibir_int_del_buffer(unBuffer);
    int direccion_fisica = __recibir_int_del_buffer(unBuffer);
    destruir_buffer(unBuffer);
    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_FS_WRITE", pid);

    log_info(io_log_debug, "RECIBI LOS VALORES: TAMANIO %ld, PUNTERO %d, DIR FISICA %d", tamanio, puntero_archivo, direccion_fisica);

    // ME FIJO EN LA TABLA Y BUSCO EL FCB ASOCIADO A ESE ARCHIVO
    pthread_mutex_lock(&mutex_lista_fat);
    t_fcb* una_fcb = obtener_fcb(nombre_archivo);
    pthread_mutex_unlock(&mutex_lista_fat);

    if(una_fcb == NULL)
    {
        pthread_mutex_lock(&mutex_lista_fat);
        crear_fcb(nombre_archivo);
        una_fcb = obtener_fcb(nombre_archivo);
        pthread_mutex_unlock(&mutex_lista_fat);
    }

    int indice = una_fcb->bloque_inicial;

    log_info(io_log_debug, "INDICE %d DE ARCHIVO %s", indice, una_fcb->nombre);

    notificar_memoria_lectura(pid, direccion_fisica, tamanio);
    sem_wait(&sem_leyo_data); 

    log_info(io_log_debug, "LLEGO LA SIGUIENTE DATA: %s", data_leida);

    escribir_data_en_bloque(indice + (puntero_archivo/BLOCK_SIZE), data_leida, tamanio);

    log_info(io_log_debug, "ESCRIBI EN EL BLOQUE");

    log_info(io_log_obligatorio, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escribir: <%ld> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);

    if(data_leida != NULL){

        free(data_leida);
        data_leida = NULL;
    }

    enviar_fin_entradasalida_a_kernel(pid);
    if(nombre_archivo != NULL){

        free(nombre_archivo);
        nombre_archivo = NULL;
    }  
}

void procesar_fs_read(t_buffer* unBuffer){
    usleep(TIEMPO_UNIDAD_TRABAJO * 1000);

    int pid = __recibir_int_del_buffer(unBuffer);
    char* nombre_archivo = __recibir_string_del_buffer(unBuffer);
    size_t tamanio = extraer_size_t_del_buffer(unBuffer);
    int puntero_archivo = __recibir_int_del_buffer(unBuffer);
    int direccion_fisica = __recibir_int_del_buffer(unBuffer);

    log_info(io_log_obligatorio, "PID: %d - Operacion: IO_FS_READ", pid);

    destruir_buffer(unBuffer);
    // ME FIJO EN LA TABLA Y BUSCO EL FCB ASOCIADO A ESE ARCHIVO
    pthread_mutex_lock(&mutex_lista_fat);
    t_fcb* una_fcb = obtener_fcb(nombre_archivo);
    pthread_mutex_unlock(&mutex_lista_fat);

    if(una_fcb == NULL)
    {
        pthread_mutex_lock(&mutex_lista_fat);
        crear_fcb(nombre_archivo);
        una_fcb = obtener_fcb(nombre_archivo);
        pthread_mutex_unlock(&mutex_lista_fat);
    }

    int indice = una_fcb->bloque_inicial;

    // LEEMOS EL DATO A PARTIR DEL INDICE + PUNTERO_ARCHIVO
    // A PARTIR DE AHI LEEMOS TAMANIO (BYTES)
    // EL VALOR OBTENIDO

    char* valor_leido_archivo;

    valor_leido_archivo = leer_data_de_bloque(indice + (puntero_archivo/BLOCK_SIZE), tamanio);

    notificar_memoria_escritura(pid, direccion_fisica, tamanio, valor_leido_archivo);
    
    log_info(io_log_obligatorio, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Escribir: <%ld> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);

    if(valor_leido_archivo != NULL){

        free(valor_leido_archivo);
        valor_leido_archivo = NULL;
    }

    enviar_fin_entradasalida_a_kernel(pid);

    if(nombre_archivo != NULL){

        free(nombre_archivo);
        nombre_archivo = NULL;
    }
}

void enviar_nombre_a_kernel(t_buffer* unBuffer) {
    char* mensaje = __recibir_string_del_buffer(unBuffer);
    destruir_buffer(unBuffer);

    t_paquete* un_paquete = __crear_super_paquete(NOMBRE_ENTRADA_SALIDA);
    if(strcmp(mensaje, "PETICION NOMBRE Y TIPO") == 0)
     {
        __cargar_string_al_super_paquete(un_paquete, NOMBRE_INTERFAZ);
        __cargar_int_al_super_paquete(un_paquete, TIPO_INTERFAZ_ENUM);
     }

    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);

    if(mensaje != NULL){

        free(mensaje);
        mensaje = NULL;
    }
}

void enviar_fin_entradasalida_a_kernel(int pid) {

    t_paquete* un_paquete = __crear_super_paquete(FIN_ENTRADASALIDA);
    __cargar_int_al_super_paquete(un_paquete, pid);
    __cargar_string_al_super_paquete(un_paquete, "OK");

    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);

    log_info(io_log_debug, "FIN ENTRADA SALIDA DE PID: %d", pid);
}

void enviar_contexto_a_kernel_con_desalojo(int pid) {

    t_paquete* un_paquete = __crear_super_paquete(FIN_ENTRADASALIDA);
    __cargar_int_al_super_paquete(un_paquete, pid);
    __cargar_string_al_super_paquete(un_paquete, "ERROR");

    enviar_paquete(un_paquete, fd_kernel);
    eliminar_paquete(un_paquete);
}

