CC=gcc
CFLAGS=-Wall -Werror -pedantic 

all: build/client build/server

build/client: build/client.o
	$(CC) $(CFLAGS) build/client.o -o build/client

build/client.o: src/client/client.c
	$(CC) $(CFLAGS) -c src/client/client.c -o build/client.o

build/server: build/server.o
	$(CC) $(CFLAGS) build/server.o -pthread -o build/server

build/server.o: src/server/server.c
	$(CC) $(CFLAGS) -c src/server/server.c -o build/server.o

clean:
	rm build/client*
	rm build/server*
	rm build/unix_udp*
	rm build/unix_udp_cl*



