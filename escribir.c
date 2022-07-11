#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "escribir.h"

int escribir_archivo(int sd, char * archivo, char * dir, struct stat * s_stat){
	//a mano se construye la informacion
	
	char filename[PATH_MAX];
	char ruta[PATH_MAX];
	
	sprintf(ruta, "%s/%s", dir, archivo);
		
	file_info info;
	
	strcpy(filename, archivo);
	
	memset(&info, 0, sizeof(file_info));
	info.size = s_stat->st_size;
	
	strcpy(info.filename, archivo);
	
	printf("Tamano: %d Archivo: %s\n",info.size,info.filename);
	
	int faltantes;
	int a_leer;
	int leidos;
	char buf[BUFSIZ];
	
	faltantes = info.size;
	
	int fd;
	int out_fd;
	
	//abrir el archivo para lectura
	fd = open(ruta, O_RDONLY);
	
	while(faltantes > 0){
		a_leer = BUFSIZ;
		
		if(faltantes < a_leer){
		   a_leer = faltantes;
		}
		//leer del archivo
		leidos = read(fd, buf, a_leer);
		//validar cuanto se leyo
   		write(sd,buf,leidos); //se escribe en el socket
		faltantes = faltantes - leidos;
	}
	printf("Archivo escrito correctamente\n");
	close(fd);
}
