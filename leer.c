#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"
#include "leer.h"


int leer_archivo(int sd, char * archivo, char *dir,struct stat * s_stat){

//a mano se construye la informacion
	
	char filename[PATH_MAX];
	char ruta[PATH_MAX];

	sprintf(ruta, "%s/%s", dir, archivo);
		
	file_info info;
	
	strcpy(filename, archivo);
	
	memset(&info, 0, sizeof(file_info));
	
	info.size = s_stat->st_size;
	
	strcpy(info.filename, archivo);
	
	printf("Tamano leer: %d Archivo: %s\n",info.size,info.filename);
	
	int faltantes;
	int a_leer;
	int leidos;
	char buf[BUFSIZ];
	
	faltantes = info.size;
	
	int fd;
	
	//abrir el archivo para lectura
	fd = open(ruta, O_RDONLY);
	
	//Construir la ruta del directorio para guardar en la carpeta 
	strcpy(filename, dir);
	strcat(filename, archivo);
	
	FILE * mihandle;
	mihandle = fopen(filename,"wb");
	memset(buf, 0, sizeof(BUFSIZ));
	
	while(faltantes > 0){
		a_leer = BUFSIZ;
		if(faltantes < BUFSIZ){
		   a_leer = faltantes;
		}
		//leer del archivo
		leidos = read(sd, buf, a_leer);
		//validar cuanto se leyo
   		fwrite(buf, 1, leidos, mihandle);
		faltantes = faltantes - leidos;
	}
	fclose(mihandle);
	printf("Archivo leido correctamente\n");
	close(fd);

}
