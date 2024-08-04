#include "../include/inicializar_entradasalida.h"
#include "../include/bitmap.h"

void inicializar(char* archivo_config)
{
    inicializar_logs();
    inicializar_configs(archivo_config);
    inicializar_semaforos();
    data_leida = NULL;

	if(strcmp(TIPO_INTERFAZ,"DIALFS") == 0){
		inicializar_estructuras_filesystem();
	}
}

void inicializar_logs(){

	io_log_debug = log_create("io_debug.log","IO_DEBUG_LOG",1,LOG_LEVEL_TRACE);

	if(io_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	io_log_obligatorio = log_create("io_log_obligatorio.log", "[IO - Log obligatorio]", 1, LOG_LEVEL_INFO);
}

void inicializar_configs(char* archivo_config){
	if((io_config = config_create(archivo_config)) == NULL)
	{
		printf("Error al crear el archivo de configuracion");
		exit(2);
	}

    TIPO_INTERFAZ = config_get_string_value(io_config, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(io_config, "TIEMPO_UNIDAD_TRABAJO");
    // IP_KERNEL = config_get_string_value(io_config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(io_config, "PUERTO_KERNEL");
    // IP_MEMORIA = config_get_string_value(io_config, "IP_MEMORIA");
    PUERTO_MEMORIA =config_get_string_value(io_config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(io_config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(io_config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(io_config, "BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(io_config, "RETRASO_COMPACTACION");
}

void inicializar_semaforos(){
    sem_init(&sem_kernel, 0, 0);
    sem_init(&sem_escribio_memoria, 0, 0);
    sem_init(&sem_leyo_data, 0, 0);
    sem_init(&sem_proceso_en_io,0,1);
}

void inicializar_estructuras_filesystem(){
    inicializar_archivo_bloques();
	inicializar_archivo_bitmap();
    inicializar_pthreads();
    lista_fat = list_create();
}

void inicializar_archivo_bloques(){
    size_t path_length = strlen(PATH_BASE_DIALFS) + strlen("bloques.dat") + 1;
    PATH_ARCHIVO_BLOQUES = malloc(path_length);
	// O_CREAT: Si el archivo no existe, se creará.
	// O_RDWR: Permite lectura y escritura en el archivo.

    if (PATH_ARCHIVO_BLOQUES == NULL) {
        log_error(io_log_debug, "Error al asignar memoria para PATH_ARCHIVO_BLOQUES");
        exit(EXIT_FAILURE);
    }

    snprintf(PATH_ARCHIVO_BLOQUES, path_length, "%sbloques.dat", PATH_BASE_DIALFS);

    printf("ARCHIVO DE BLOQUES: %s\n", PATH_ARCHIVO_BLOQUES);

    fd_archivoBloques = open(PATH_ARCHIVO_BLOQUES, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	// CREAR EL fd_archivoBloques
	int tamanio_archivo_bloques = BLOCK_SIZE * BLOCK_COUNT;
	ftruncate(fd_archivoBloques, tamanio_archivo_bloques);


	// MAPEO EL ARCHIVO A UN VOID*. AMBOS ESTÁN RELACIONADOS.
	bloquesEnMemoria = mmap(NULL, BLOCK_SIZE * BLOCK_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, fd_archivoBloques, 0);
	if (bloquesEnMemoria == MAP_FAILED) {
		log_error(io_log_debug, "Error al mapear los bloques en memoria");
		exit(EXIT_FAILURE);
	}

    log_info(io_log_debug, "Archivo de bloques inicializado correctamente.\n");
}

void inicializar_archivo_bitmap() {
	size_t path_length = strlen(PATH_BASE_DIALFS) + strlen("bitmap.dat") + 1;
    PATH_BITMAP = malloc(path_length);

    if (PATH_BITMAP == NULL) {
        log_error(io_log_debug,"Error al asignar memoria para PATH_BITMAP");
        exit(EXIT_FAILURE);
    }

    snprintf(PATH_BITMAP, path_length, "%sbitmap.dat", PATH_BASE_DIALFS);

    fd_bitmap = open(PATH_BITMAP, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1) {
        log_error(io_log_debug, "Error al abrir/crear el archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    int tamanio_bitmap = (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0); // En bytes
    if (ftruncate(fd_bitmap, tamanio_bitmap) == -1) {
        log_error(io_log_debug, "Error al definir el tamaño del archivo de bitmap");
        close(fd_bitmap);
        exit(EXIT_FAILURE);
    }

    bitmapChar = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (bitmapChar == MAP_FAILED) {
        log_error(io_log_debug, "Error al mapear el archivo de bitmap en memoria");
        close(fd_bitmap);
        exit(EXIT_FAILURE);
    }

    bitmap = bitarray_create_with_mode(bitmapChar, tamanio_bitmap, LSB_FIRST);
    imprimir_bitarray(bitmap);

    log_info(io_log_debug, "Archivo de bitmap inicializado correctamente.\n");
}

void inicializar_pthreads(){
    pthread_mutex_init(&mutex_bloquesEnMemoria, NULL);
    pthread_mutex_init(&mutex_lista_fat, NULL);
    pthread_mutex_init(&mutex_bitmap, NULL);
}


