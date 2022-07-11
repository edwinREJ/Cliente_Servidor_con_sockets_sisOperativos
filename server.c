#include <arpa/inet.h>
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
#include "escribir.h"
#include "leer.h"
#include "netutil.h"

//Socket del servidor
int s;
//Socket del cliente
int c;

// Definir un tipo fachada para los socket
typedef int socket_t;

int finished = 0;
//crear el semaforo
sem_t mutex;

//array de identificadores de clientes.
socket_t *clients;

//array de hilos de lectura (apuntadores)
pthread_t *thrs_read;
int nClients = 0;
int size = 0;

//procedimiento que envia desde el servidor al cliente una bandera 1 si fue un exito 
void enviar_bandera(int client_sd);

//funcion que captura una interrupción del sistema
void handle_sigterm(int sig);

//Método que elimina clientes del array de apuntadores
void eliminar_cliente(int c);
void *leercabecera(void *client_socket);

//método que agrega clientes al array de apuntadores
int agregar_cliente(int client_socket);

int finished;
int pos=0;
int puerto=0;

int main(int argc, char * argv[]){

	/*
	argv[0] = ejecutable
   	argv[1] = puerto
	*/

	//Direccion del servidor IPv4
	struct sockaddr_in *addr;
	
	//Socket IPv4, de tipo flujo(stream)
	//1 Crear un socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	
	if(s < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (argc != 2)
	  {
	    fprintf(stderr, "Ingresar el puerto a escuchar\n");
	    exit(EXIT_FAILURE);
	  }
	
	//Preparar la dirrecion para asociar el socket
	 memset(&addr, 0, sizeof(struct sockaddr_in));
  	 addr = server_address(atoi(argv[1]));
	
	//2 Asociar el socket a una direccion IPv4
	 int r = bind(s, (struct sockaddr *)addr, sizeof(struct sockaddr_in));

	  if (r < 0)
	  {
	    perror("bind");
	    exit(EXIT_FAILURE);
	  }
	
	//3 Poner el socket listo
	 if (listen(s, 10) < 0)
	  {
	    perror("listen");
	    exit(EXIT_FAILURE);
	  }
	
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len;
	client_addr_len = sizeof(struct sockaddr_in);//Tamaño espererado de la direccion
	
	// Reservar la memoria para el array clients
	clients = (socket_t *)malloc(size * sizeof(socket_t));

	memset(clients, -1, size);

	int count = 0;

	sem_init(&mutex, 0, 1);

	// Reservar la memoria para el array de hilos de lectura
	thrs_read = (pthread_t *)malloc(size * sizeof(pthread_t));

	// Definir el manejador de sigterm
	signal(SIGTERM, handle_sigterm);
	
	finished = 0;
	
	
	int i = 0;
	int newclient;

  	// Se atendiende todas las peticiones realizadas por el client 
  	
  	while (!finished)
  	{
   	  //Se bloquea hasta que se conecte un client
   	   printf("Esperando una conexion\n"); 
  	   newclient = accept(s, 0, 0);
           
   	  // Si se ha recibido una señal de terminación: finished = 1
  	  if (finished)
   	  {
   	    break;
   	  }

	  if (newclient < 0)
	  {
	     perror("accept");
	     continue;
	  }
	 
	  sem_wait(&mutex);
	  sem_post(&mutex); 
	  pos = agregar_cliente(newclient);
	 
	  printf("Cliente %d se ha conectado al servidor\n",pos+1);
	  pthread_create(
		&thrs_read[pos],      // Referencia en donde se almacena el nuevo id
		NULL,                 // Atributos por defecto
		leercabecera,         // Funcion que se ejecutara dentro del nuevo hilo
		(void *)&clients[pos] // Argumento que recibe el hilo
	  );
	     
        }

	//se libera memoria de las variables utilizadas
	free(clients);
	free(thrs_read);	
	exit(EXIT_SUCCESS);
}


void *leercabecera(void *client_socket)
{
  file_info archivo;
  int finalizo = 0;
  int faltante = 0;
  char buf[BUFSIZ];
  int a_leer;
  int acread;
  int client_sd = *(int *)client_socket;
  request r;
  char ruta[PATH_MAX];
  struct stat s_stat;
  
   
  while (!finalizo)
  { 
    int nread = read(client_sd, (char*)&r, sizeof(request));
    
    if (nread == 0)
    {
      printf("Cliente %d ha finalizado el proceso\n",pos+1);
      sem_wait(&mutex);
      eliminar_cliente(client_sd);
      sem_post(&mutex);
      break;
    }
     
    if (strcmp(r.filename, "") == 0){
   	 printf("Solicitud: operacion: %s\n",r.operation);
    }
    else{
    	printf("Solicitud: operacion: %s archivo: %s\n",r.operation,r.filename);
    }
      
    if(EQUALS(r.operation, "get")){
    
    //Construir la ruta
    sprintf(ruta, "cap_server/%s", r.filename);
    
    if(stat(ruta,&s_stat) < 0){
        perror("stat");
        continue;
     }  
     //verificar si es un archivo regular
     if(!S_ISREG(s_stat.st_mode)){
	printf("%s no es un archivo\n",ruta);
	continue;
     }
    escribir_archivo(client_sd,r.filename,"cap_server",&s_stat);
   	 
    }else if(EQUALS(r.operation, "put")){	
    
       //Construir la ruta
       sprintf(ruta, "cap_client/%s", r.filename);
    
       if(stat(ruta,&s_stat) < 0){
        perror("stat");
        continue;
       }     
       //verificar si es un archivo regular
       if(!S_ISREG(s_stat.st_mode)){
	   printf("%s no es un archivo\n",ruta);
	   continue;
       }    	
       leer_archivo(client_sd,r.filename,"cap_server/",&s_stat); 
    }
    
   continue;
  }
}

int agregar_cliente(int client_socket)
{
  if (nClients == size)
  {

    //Calcular el nuevo tamaño
    int nSize = (size * 3) / 2;
    if (nSize <= 1)
    {
      nSize = 2;
    }

    // Reservar memoria para el nuevo arreglo
    socket_t *new_clients = (socket_t *)malloc(nSize * sizeof(socket_t));
    pthread_t *new_thrs_read = (pthread_t *)malloc(nSize * sizeof(pthread_t));

    for (int i = 1; i <= size; i++)
    {
      new_clients[i] = clients[i];
      new_thrs_read[i] = thrs_read[i];
    }

    // Liberar el arreglo viejo
    free(clients);
    free(thrs_read);

    // Asignar el arreglo nuevo
    clients = new_clients;
    thrs_read = new_thrs_read;

    // Actualizar el nuevo tamaño
    size = nSize;
  }
  clients[nClients] = client_socket;
  int pos = nClients;
  nClients++;
  
  return pos;
}

void handle_sigterm(int sig)
{
  finished = 1;

  // Cerrar todos los sockets de clientes
  for (int i = 0; i < nClients; i++)
  {
    if (clients[i] != -1)
    {
      sem_wait(&mutex);
      eliminar_cliente(clients[i]);
      sem_post(&mutex);
    }
  }

  // Cerrar socket del servidor
  close(s);

  // Mandarme una señal de terminación con el manejador por defecto
  signal(SIGTERM, SIG_DFL);
  //kill(getpid(), SIGTERM);
}

void eliminar_cliente(int c)
{
  // cerrar el socket c
  close(c);
  int i;

  // buscar en el arreglo de lcientes el id = c y colocar -1
  for (i = 0; i < nClients; i++)
  {
    if (clients[i] == c)
    {
      clients[i] = -1;
      break;
    }
  }

  // correr los clientes hacia arriba una posición desde c
  for (int j = i; j < nClients; j++)
  {
    if (clients[j + 1] != -1)
    {
      clients[j] = clients[j + 1];
      clients[j + 1] = -1;
    }
  }

  // restarle 1 a nclients
  nClients--;
}

