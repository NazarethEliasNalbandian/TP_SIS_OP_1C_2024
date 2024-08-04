#include "../include/dic_operaciones.h"

void diccionario_operaciones(){
	op_autorizada = string_array_new();
	string_array_push(&op_autorizada, "SET");
	string_array_push(&op_autorizada, "SUM");
	string_array_push(&op_autorizada, "SUB");
	string_array_push(&op_autorizada, "JNZ");
	string_array_push(&op_autorizada, "IO_GEN_SLEEP");
	string_array_push(&op_autorizada, "RESIZE");
	string_array_push(&op_autorizada, "WAIT");
	string_array_push(&op_autorizada, "SIGNAL");
	string_array_push(&op_autorizada, "MOV_IN");
	string_array_push(&op_autorizada, "MOV_OUT");
	string_array_push(&op_autorizada, "COPY_STRING");
	string_array_push(&op_autorizada, "IO_STDIN_READ");
	string_array_push(&op_autorizada, "IO_STDOUT_WRITE");
	string_array_push(&op_autorizada, "IO_FS_CREATE");
	string_array_push(&op_autorizada, "IO_FS_DELETE");
    string_array_push(&op_autorizada, "IO_FS_TRUNCATE");
    string_array_push(&op_autorizada, "IO_FS_WRITE");
	string_array_push(&op_autorizada, "IO_FS_READ");
	string_array_push(&op_autorizada, "EXIT");
}