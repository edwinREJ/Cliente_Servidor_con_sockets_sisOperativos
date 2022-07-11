#include <limits.h>
//no me dejo utilizar los guardas 
typedef struct{
	char operation[16];
	char filename[PATH_MAX];
}request;

typedef struct{
	int size; // tamaño del archivo a transferir, -1 si no existe
	char filename[PATH_MAX];
	int bandera; // 1 cuando el envio del archivo es exitoso 
}file_info;

#define EQUALS(s1, s2) (strcmp(s1, s2) == 0)
