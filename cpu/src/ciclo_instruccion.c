#include "../include/ciclo_instruccion.h"
#include "../include/cpu_kernel_dispatch.h"
#include "../include/mmu.h"
#include "../include/servicios_cpu.h"

void fetch()
{
    // pedido de instrucciones a memoria
    log_info(cpu_log_obligatorio, "PID: <%d> - FETCH - Program Counter: <%d>", un_contexto->pID, un_contexto->r_cpu->PC);
    t_paquete *paquete = crear_super_paquete(PETICION_DE_INSTRUCCIONES_CPU_MEMORIA);
    cargar_int_al_super_paquete(paquete, un_contexto->pID);
    cargar_uint32_al_super_paquete(paquete, un_contexto->r_cpu->PC);
    enviar_paquete(paquete, fd_memoria);
    eliminar_paquete(paquete);
}

void decode()
{
    pthread_mutex_lock(&mutex_instruccion_split);
	if(validador_de_header(instruccion_split[0])){
		// log_info(cpu_logger, "Instruccion Validada: [%s] -> OK", instruccion_split[0]);
		sem_post(&sem_decode);
	}else{
		log_error(cpu_log_obligatorio, "Instruccion no encontrada: [PC: %d][Instruc_Header: %s]", un_contexto->r_cpu->PC, instruccion_split[0]);
		desalojar = true;
        tipo_desalojo = ATENDER_EXIT;
        flag_exit = true; //[FALTA] Repensar como terminar el programa y destruir estructuras
	}
    pthread_mutex_unlock(&mutex_instruccion_split);
}


uint32_t solicitar_uint32_memoria(int dir_logica)
{
    int dir_fisica = MMU(dir_logica);
    size_t tam=sizeof(u_int32_t);

    if (dir_fisica == -1)
    {
        return -1;
    }
    else
    {
        t_paquete *paqueteLecturaMemoria = crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, un_contexto->pID);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, dir_fisica);
        cargar_size_t_al_super_paquete(paqueteLecturaMemoria, tam);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, T_UINT32);

        enviar_paquete(paqueteLecturaMemoria, fd_memoria);
        eliminar_paquete(paqueteLecturaMemoria);

        sem_wait(&sem_val_leido);

        log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%u>", un_contexto->pID, dir_fisica, valor_leido_uint32);

        return valor_leido_uint32;
    }
}

uint8_t solicitar_uint8_memoria(int dir_logica)
{
    int dir_fisica = MMU(dir_logica);
    size_t tam=sizeof(uint8_t);

    if (dir_fisica == -1)
    {
        return -1;
    }
    else
    {
        t_paquete *paqueteLecturaMemoria = crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, un_contexto->pID);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, dir_fisica);
        cargar_size_t_al_super_paquete(paqueteLecturaMemoria, tam);
        cargar_int_al_super_paquete(paqueteLecturaMemoria, T_UINT8);

        enviar_paquete(paqueteLecturaMemoria, fd_memoria);
        eliminar_paquete(paqueteLecturaMemoria);

        sem_wait(&sem_val_leido);

        log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%u>", un_contexto->pID, dir_fisica, valor_leido_uint8);

        return valor_leido_uint8;
    }
}

char* solicitar_string_memoria(uint32_t dir_logica, int tamanio)
{
    size_t tam=(size_t)tamanio;
    //int cant_pags=ceil(tamanio/TAM_PAGINA);
    uint32_t aux=dir_logica;
  //  while(cant_pags!=0){
    int dir_fisica = MMU(aux);
        if (dir_fisica == -1)
        {
            return "ERROR";
        }
        else
        {
            // Le pido a memoria el contenido del marco
            t_paquete *paqueteLecturaMemoria = crear_super_paquete(LECTURA_BLOQUE_CPU_MEMORIA);
            cargar_int_al_super_paquete(paqueteLecturaMemoria, un_contexto->pID);
            cargar_int_al_super_paquete(paqueteLecturaMemoria, dir_fisica);
            cargar_size_t_al_super_paquete(paqueteLecturaMemoria, tam);
            cargar_int_al_super_paquete(paqueteLecturaMemoria, T_STRING);

            enviar_paquete(paqueteLecturaMemoria, fd_memoria);
            eliminar_paquete(paqueteLecturaMemoria);

            // esperar rta de memoria

            sem_wait(&sem_val_leido);

            log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s> ", un_contexto->pID, dir_fisica, valor_leido_string-1);
        }

    return valor_leido_string;
}

int escribir_valor_memoria_uint8(uint32_t dir_logica, uint8_t valorAEscribir)
{
    int dir_fisica = MMU(dir_logica);
    size_t tam = sizeof(uint8_t);

    if (dir_fisica != -1)
    {

        t_paquete *paqueteEscrituraMemoria = crear_super_paquete(ESCRITURA_BLOQUE_CPU_MEMORIA);

        cargar_int_al_super_paquete(paqueteEscrituraMemoria, un_contexto->pID);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, dir_fisica);
        cargar_size_t_al_super_paquete(paqueteEscrituraMemoria, tam);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, T_UINT8);
        cargar_choclo_al_super_paquete(paqueteEscrituraMemoria, &valorAEscribir, tam);

        enviar_paquete(paqueteEscrituraMemoria, fd_memoria);
        eliminar_paquete(paqueteEscrituraMemoria);

        sem_wait(&sem_val_escrito);

        log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", un_contexto->pID, dir_fisica, valorAEscribir);
        return 0;
    }
    else{
        return -1;
    }
}

int escribir_valor_memoria_uint32(uint32_t dir_logica, uint32_t valorAEscribir)
{
    int dir_fisica = MMU(dir_logica);
    size_t tam = sizeof(uint32_t);

    if (dir_fisica != -1)
    {

        t_paquete *paqueteEscrituraMemoria = crear_super_paquete(ESCRITURA_BLOQUE_CPU_MEMORIA);

        cargar_int_al_super_paquete(paqueteEscrituraMemoria, un_contexto->pID);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, dir_fisica);
        cargar_size_t_al_super_paquete(paqueteEscrituraMemoria, tam);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, T_UINT32);
        cargar_choclo_al_super_paquete(paqueteEscrituraMemoria,  &valorAEscribir, tam);

        enviar_paquete(paqueteEscrituraMemoria, fd_memoria);
        eliminar_paquete(paqueteEscrituraMemoria);

        sem_wait(&sem_val_escrito);

        log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", un_contexto->pID, dir_fisica, valorAEscribir);
        return 0;
    }
    else{
        return -1;
    }
}

int escribir_valor_memoria_string(uint32_t dir_logica, char* valorAEscribir)
{

    if (valorAEscribir == NULL) {
        log_error(cpu_log_obligatorio, "Error: valorAEscribir es NULL");
        return -1;
    }
    
    int dir_fisica = MMU(dir_logica);
    size_t tam = strlen(valorAEscribir);

    if (dir_fisica != -1)
    {

        t_paquete *paqueteEscrituraMemoria = crear_super_paquete(ESCRITURA_BLOQUE_CPU_MEMORIA);

        cargar_int_al_super_paquete(paqueteEscrituraMemoria, un_contexto->pID);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, dir_fisica);
        cargar_size_t_al_super_paquete(paqueteEscrituraMemoria, tam);
        cargar_int_al_super_paquete(paqueteEscrituraMemoria, T_STRING);
        cargar_choclo_al_super_paquete(paqueteEscrituraMemoria, (void*) valorAEscribir, tam);

        enviar_paquete(paqueteEscrituraMemoria, fd_memoria);
        eliminar_paquete(paqueteEscrituraMemoria);


        sem_wait(&sem_val_escrito);

        log_info(cpu_log_obligatorio, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", un_contexto->pID, dir_fisica, valorAEscribir);
        free(valorAEscribir); 
        return 0;
    }
    else{
        return -1;
    }
}


void execute()
{
    char* nombre_instruccion = instruccion_split[0]; 

    nombre_instruccion_enum = convertirStringAEnum(nombre_instruccion);

    int cantidadParametros;
    t_paquete * paquete = NULL;
    int dir_logica;
    int dir_fisica;
    size_t tamanio;
    int puntero;
    int resultado;
    uint32_t *er = NULL;
    uint8_t *r  = NULL;
    uint32_t *era = NULL;
    uint32_t *erb = NULL;
    uint8_t *ra = NULL;
    uint8_t *rb = NULL;
    u_int32_t* registroAEscribirUint32 = NULL;
    uint8_t* registroAEscribirUint8  = NULL;
    uint32_t valor_a_escribir_uint32;
    uint8_t valor_a_escribir_uint8;

    switch (nombre_instruccion_enum)
    {
        case SET:
            // SET(Registro, Valor)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            if (es_registro_uint32(instruccion_split[1]))
            {
                er = detectar_registroE(instruccion_split[1]);
                *er = atoi(instruccion_split[2]);
            }
            else
            {
                r = detectar_registro(instruccion_split[1]);
                *r = atoi(instruccion_split[2]);
            }
            break;
        case SUM:
            // SUM (Registro Destino, Registro Origen)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            if (es_registro_uint32(instruccion_split[1]))
            {
                era = detectar_registroE(instruccion_split[1]);
                erb = detectar_registroE(instruccion_split[2]);
                *era = (*era) + (*erb);
            }
            else
            {
                ra = detectar_registro(instruccion_split[1]);
                rb = detectar_registro(instruccion_split[2]);
                *ra = (*ra) + (*rb);
            }
            break;
        case SUB:
            // SUB (Registro Destino, Registro Origen)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            if (es_registro_uint32(instruccion_split[1]))
            {
                era = detectar_registroE(instruccion_split[1]);
                erb = detectar_registroE(instruccion_split[2]);
                *era = (*era) - (*erb);
            }
            else
            {
                ra = detectar_registro(instruccion_split[1]);
                rb = detectar_registro(instruccion_split[2]);
                *ra = (*ra) - (*rb);
            }
            break;
        case JNZ:
            // JNZ (Registro Destino, Registro Origen)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            if (es_registro_uint32(instruccion_split[1]))
            {
                er = detectar_registroE(instruccion_split[1]);
                
                if (*er != 0)
                {
                    un_contexto->r_cpu->PC = (u_int32_t) atoi(instruccion_split[2]);
                }
            }
            else
            {
                r = detectar_registro(instruccion_split[1]);
                
                if (*r != 0)
                {
                    un_contexto->r_cpu->PC = (u_int32_t) atoi(instruccion_split[2]);
                }
            }
            break;
        case IO_GEN_SLEEP:
            // IO_GEN_SLEEP(Interfaz, Unidades de trabajo)
            un_contexto->r_cpu->PC+= 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cargar_int_al_super_paquete(paquete, 2); 

            // PARAMETROS

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, atoi(instruccion_split[2])); 

            enviar_paquete(paquete, fd_kernel_dispatch);

            eliminar_paquete(paquete);

            

            break;
        case RESIZE:
            // RESIZE (Tamaño)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1]);
            int t = atoi(instruccion_split[1]);

            paquete = crear_super_paquete(RESIZE_CPU_MEMORIA);

            cargar_int_al_super_paquete(paquete, un_contexto->pID); 
            cargar_int_al_super_paquete(paquete, t); 

            enviar_paquete(paquete, fd_memoria);
            sem_wait(&sem_resize);
            if(strcmp(rta_m_resize,"OUT OF MEMORY") == 0)
            {
                desalojar = true;
                tipo_desalojo = ATENDER_OUT_OF_MEMORY;
            }
            if(rta_m_resize != NULL){
                free(rta_m_resize);
                rta_m_resize = NULL;
            }
            eliminar_paquete(paquete);
            break;
        case WAIT:
            // WAIT
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1]);


            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 1;
            cargar_int_al_super_paquete(paquete, cantidadParametros); 

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE RECURSO

            enviar_paquete(paquete, fd_kernel_dispatch);

            sem_wait(&sem_rta_kernel);

            eliminar_paquete(paquete);
            break;
        case SIGNAL:
            // SIGNAL
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1]);

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 1;
            cargar_int_al_super_paquete(paquete, cantidadParametros); 

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE RECURSO

            enviar_paquete(paquete, fd_kernel_dispatch);

            sem_wait(&sem_rta_kernel);

            eliminar_paquete(paquete);
            break;
        case MOV_IN:
            // MOV_IN (Registro Datos, Registro Dirección)
            un_contexto->r_cpu->PC+= 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            dir_logica = obtenerIntDeRegistro(instruccion_split[2],&er,&r);

            if(es_registro_uint32(instruccion_split[1])){
               valor_a_escribir_uint32 = solicitar_uint32_memoria(dir_logica);
               escribir_registro(instruccion_split[1], valor_a_escribir_uint32, registroAEscribirUint32, registroAEscribirUint8);
            }else if(es_registro_uint8(instruccion_split[1])){
                valor_a_escribir_uint8 = solicitar_uint8_memoria(dir_logica);
                escribir_registro(instruccion_split[1], valor_a_escribir_uint8, registroAEscribirUint32, registroAEscribirUint8);
            }
            
            break;
        case MOV_OUT:
            // MOV_OUT (Registro Dirección, Registro Datos)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            
            dir_logica = obtenerIntDeRegistro(instruccion_split[1], &er, &r);


            if(es_registro_uint32(instruccion_split[2])){
                asignarRegistroExtended(instruccion_split[2], &registroAEscribirUint32);
                valor_a_escribir_uint32 = *registroAEscribirUint32;
                log_trace(cpu_log_debug,"VALOR A ESCRIBIR: %d", (int) valor_a_escribir_uint32);
                // resultado = escribir_valor_memoria_string((u_int32_t) dir_logica, (char*) valor_a_escribir_uint32);
                resultado = escribir_valor_memoria_uint32((uint32_t) dir_logica, valor_a_escribir_uint32);
            } else if (es_registro_uint8(instruccion_split[2])){
                asignarRegistro(instruccion_split[2], &registroAEscribirUint8);
                valor_a_escribir_uint8 = *registroAEscribirUint8;
                log_trace(cpu_log_debug,"VALOR A ESCRIBIR: %d", (int) valor_a_escribir_uint8);
                // resultado = escribir_valor_memoria_string((u_int32_t) dir_logica, (char*) valor_a_escribir_uint8);
                resultado = escribir_valor_memoria_uint8((uint32_t) dir_logica, valor_a_escribir_uint8);
            }

            if(resultado!=0){
                log_error(cpu_log_debug, "FALLO EN ESCRITURA");
                exit(EXIT_FAILURE);
            }
            
            break;
        case COPY_STRING:
            // COPY_STRING (Tamaño)
            // puede ocupar mas de 1 pagina el string
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1]);
            uint32_t si = un_contexto->r_cpu->SI;
            int tam = atoi(instruccion_split[1]);
            char* valor_str = solicitar_string_memoria(si,tam);
            if(strcmp(valor_str,"ERROR") != 0){
                char* aux = string_substring(valor_str, 0, tam);
                uint32_t di = un_contexto->r_cpu->DI;
                int resultado = escribir_valor_memoria_string(di, aux);
                // if(aux != NULL)
                // {
                //     free(aux);
                //     aux = NULL;
                // }
                if(resultado!=0){
                    desalojar = true;
                tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
                }
            }
            else{
                desalojar = true;
                tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
            }
            if(valor_str != NULL){
                free(valor_str);
                valor_str = NULL;
            }
            break;
        case IO_STDIN_READ:
            // IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
            // puede ocupar mas de 1 pagina el string
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3]);
            

            tamanio = obtenerSizeTDeRegistro(instruccion_split[3], &er, &r);

            log_trace(cpu_log_debug,"tamanio: %ld", tamanio);
            dir_logica = obtenerIntDeRegistro(instruccion_split[2], &er, &r);

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 3;
            cargar_int_al_super_paquete(paquete, cantidadParametros);
            
            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_super_paquete(paquete, tamanio);

            dir_fisica = MMU(dir_logica);
            if(dir_fisica == -1){
                desalojar = true;
                 tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
            }
            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, dir_fisica);

            if(desalojar){
                eliminar_paquete(paquete);
                break;
            }
            else{
                enviar_paquete(paquete, fd_kernel_dispatch);

            }

            eliminar_paquete(paquete);
            break; 
        case IO_STDOUT_WRITE:
            // IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)
            // puede ocupar mas de 1 pagina el string
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3]);
            
            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            tamanio = obtenerSizeTDeRegistro(instruccion_split[3], &er, &r);
            dir_logica = obtenerIntDeRegistro(instruccion_split[2], &er, &r);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]);
            cantidadParametros = 3;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_super_paquete(paquete, tamanio);
            
            dir_fisica = MMU(dir_logica);
            if(dir_fisica == -1){
                desalojar = true;
                tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
            }
            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, dir_fisica);
            
            if(desalojar){
                eliminar_paquete(paquete);
                break;
            }
            else{
                enviar_paquete(paquete, fd_kernel_dispatch);
            }

            eliminar_paquete(paquete);
            break;
        case IO_FS_CREATE:
            // IO_FS_CREATE (Interfaz, Nombre Archivo)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);


            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 2;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[2]); // NOMBRE ARCHIVO

            enviar_paquete(paquete, fd_kernel_dispatch);

            eliminar_paquete(paquete);
            break;
        case IO_FS_DELETE:
            // IO_FS_DELETE (Interfaz, Nombre Archivo)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 2;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[2]); // NOMBRE ARCHIVO

            enviar_paquete(paquete, fd_kernel_dispatch);

            eliminar_paquete(paquete);
            break;
        case IO_FS_TRUNCATE:
            // IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3]);

            tamanio = obtenerSizeTDeRegistro(instruccion_split[3], &er, &r);

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 3;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[2]); // NOMBRE ARCHIVO

            cargar_int_al_super_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_super_paquete(paquete, tamanio); // TAMAÑO

            enviar_paquete(paquete, fd_kernel_dispatch);

            eliminar_paquete(paquete);
            break;
        case IO_FS_WRITE:
            // IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            // puede ocupar mas de 1 pag el string y escribir en un pos
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3], instruccion_split[4], instruccion_split[5]);
            
            tamanio = obtenerSizeTDeRegistro(instruccion_split[4], &er, &r);   

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 5;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[2]); // NOMBRE ARCHIVO
            
            puntero = obtenerIntDeRegistro(instruccion_split[5], &er, &r);

            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, puntero); //PUNTERO

            cargar_int_al_super_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_super_paquete(paquete, tamanio);

            dir_logica = obtenerIntDeRegistro(instruccion_split[3], &er, &r);

            dir_fisica = MMU(dir_logica);
            if(dir_fisica == -1){
                desalojar = true;
                tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
            }
            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, dir_fisica);
            if(desalojar){
                eliminar_paquete(paquete);
                break;
            }
            else{
                enviar_paquete(paquete, fd_kernel_dispatch);

            }

            eliminar_paquete(paquete);
            break;
        case IO_FS_READ:
            // IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s> - <%s> - <%s>", un_contexto->pID, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3], instruccion_split[4], instruccion_split[5]);
            
            tamanio = obtenerSizeTDeRegistro(instruccion_split[4], &er, &r);  

            paquete = crear_super_paquete(ATENDER_INSTRUCCION_CPU);

            agregar_contexto_a_paquete(paquete);

            cargar_string_al_super_paquete(paquete, instruccion_split[0]); // NOMBRE INSTRUCCION
            cantidadParametros = 5;
            cargar_int_al_super_paquete(paquete, cantidadParametros);

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[1]); // NOMBRE INTERFAZ

            cargar_int_al_super_paquete(paquete, T_STRING);
            cargar_string_al_super_paquete(paquete, instruccion_split[2]); // NOMBRE ARCHIVO

            puntero = obtenerIntDeRegistro(instruccion_split[5], &er, &r);

            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, puntero);     // PUNTERO

            cargar_int_al_super_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_super_paquete(paquete, tamanio);  // TAMAÑO
       
            dir_logica = obtenerIntDeRegistro(instruccion_split[3], &er, &r);

            dir_fisica = MMU(dir_logica);
            if(dir_fisica == -1){
                desalojar = true;
                tipo_desalojo = ATENDER_EXIT;
                flag_exit = true;
            }
            cargar_int_al_super_paquete(paquete, T_INT);
            cargar_int_al_super_paquete(paquete, dir_fisica);

            if(desalojar){
                eliminar_paquete(paquete);
                break;
            }
            else{
                enviar_paquete(paquete, fd_kernel_dispatch);

            }

            eliminar_paquete(paquete);
            break;
        case EXITT:
            // EXIT
            un_contexto->r_cpu->PC += 1;
            log_info(cpu_log_obligatorio, "PID: <%d> - Ejecutando: <%s>", un_contexto->pID, instruccion_split[0]);

            desalojar = true;
            tipo_desalojo = ATENDER_EXIT;
            flag_exit = true;
            break;
    }
    // if(valor_leido_string != NULL)
    // {
    //     free(valor_leido_string);
    //     valor_leido_string = NULL;
    // }
 
    if(instruccion_split != NULL){

        // liberar_instruccion_split(instruccion_split);
        string_array_destroy(instruccion_split);
        instruccion_split = NULL;
    }
}