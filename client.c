#include <arpa/inet.h> //inet_aton, inet_ntoa, ...
#include <netinet/in.h> //IPv4
#include <pthread.h> // Hilos
#include <semaphore.h> //Semaforos
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h> //Sockets
#include <unistd.h>
#include "protocol.h"
#include "split.h"
#include "escribir.h"
#include "leer.h"
#include "netutil.h"

int finished;

int main(int argc, char * argv[]){

	//Socket del servidor
	int s;
	
	//Direccion del servidor IPv4
	struct sockaddr_in *addr;
	struct stat s_stat;
	char comando[BUFSIZ];
	char archi[BUFSIZ];
   	char *command;
  	FILE *fp;
   	split_list *sp;
   	char *filename;
   	char *path;
   	int puerto;
   	char ruta[PATH_MAX];
   	
   	/* 
   		argv[0] = ejecutable
   		argv[1] = ip
   		argv[2] = puerto
   	*/
	
	 /* Primero se obtiene la direccion IP del host a conectar */
	   if (argc != 3)
	   {
	      printf("\nFaltan parametros de llamada.\nUso:\n\n%s nombre_host\n\n", argv[0]);
	      exit(EXIT_FAILURE);
	   }

	   puerto = atoi(argv[2]);

	   if (puerto < 1024 || puerto > 65535)
	   {
	      fprintf(stderr, "Puerto fuera del rango");
	      exit(EXIT_FAILURE);
	   }
	
	//Socket IPv4, de tipo flujo(stream)
	//1 Crear un socket del servidor
	s = socket(AF_INET, SOCK_STREAM, 0);
	
	if(s < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	//Preparar la dirrecion para asociar el socket
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	//asignacion de direccion ip y puerto por entrada de comandos
   	addr = address_by_ip(argv[1], atoi(argv[2]));
	
	printf("Conectando..\n");
	
	//2 Conectarse al servidor
	int r = connect(s,(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	
	//Se valida si la conxion fue exitosa.
	 if (r < 0)
	 {
	      perror("Conexion");
	      exit(EXIT_FAILURE);
	 }
	printf("Conectado\n");
	
	/* Recibir comandos del usuario */
  	 while (1)
 	  {
 	  	/* Leer el comando por entrada estandar */
	      printf(">:");
	      /*Operacion bloqueante*/
	      if (fgets(comando, BUFSIZ, stdin) == NULL)
	      {
		 perror("fgets");
		 continue;
	      }
	     
	      //leer el comando y partir
	      //Enviar la solicitud al servidor
	      request r;
	      file_info archivo;
	      memset(&r, 0, sizeof(request));
	      memset(&archivo, 0, sizeof(archivo));
	      
	      sp = split(comando, " \r\n");
	      
	      if (sp->count == 0)
	      {
		 continue;
	      }
	      if (strcmp(sp->parts[0], "exit") == 0)
	      {
	      	 command = sp->parts[0];
	      	 strcpy(r.operation, command);
	         if (write(s, (char*)&r, sizeof(request)) == -1) {
			perror("Write failed");
			continue;
		 }
		 exit(EXIT_SUCCESS);
	      }
	      else if (strcmp(sp->parts[0], "put") == 0)
	      {
		
		 filename = sp->parts[1];
		 command = sp->parts[0];
					 
		 //Construir la ruta
		 sprintf(ruta, "cap_client/%s", filename);
		 if(stat(ruta,&s_stat) < 0){
		    perror("stat");
		    continue;
	         }
		 
		 //construir la solicitud
		 strcpy(r.operation, command);
	    	 strcpy(r.filename, filename);
	    	 
	    	 //se escribe en el socket
	    	 if (write(s, (char*)&r, sizeof(request)) == -1) {
			perror("Write failed");
			continue;
		 }
		 escribir_archivo(s,filename,"cap_client",&s_stat);
	    		
		 continue;
	      }
	      else if (strcmp(sp->parts[0], "get") == 0)
	      {
	         filename = sp->parts[1];
		 command = sp->parts[0];
	         //Construir la ruta
		 sprintf(ruta, "cap_server/%s", filename);
		 if(stat(ruta,&s_stat) < 0){
		    perror("stat");
		    continue;
	         }
	         int tama = s_stat.st_size;
	         printf("tamanoooooo: %d\n",tama);
		 strcpy(r.operation, command);
	    	 strcpy(r.filename, filename);
	      
	          //se escribe en el socket
	    	 if (write(s, (char*)&r, sizeof(request)) == -1) {
			perror("Write failed");
			continue;
		 }
		 leer_archivo(s,filename,"cap_client/",&s_stat);
		 continue;
	      }
	      else
	      {
		 printf("El comando ingresado no es valido\n");
		 continue;
	      }
	   }
	   close(s);
	
	exit(EXIT_SUCCESS);
}
