all: server.o client.o split.o escribir.o netutil.o leer.o
	gcc -o server server.o split.o escribir.o leer.o netutil.o -pthread
	gcc -o client client.o split.o escribir.o leer.o netutil.o -pthread
server.o: server.c
	gcc -c -o server.o server.c
client.o: client.c
	gcc -c -o client.o client.c
split.o: split.c
	gcc -c -o split.o split.c
escribir.o: escribir.c
	gcc -c -o escribir.o escribir.c	
leer.o: leer.c
	gcc -c -o leer.o leer.c 	
netutil.o: netutil.c
	gcc -c -o netutil.o netutil.c		
clean:
	rm -rf *.o server client files/
