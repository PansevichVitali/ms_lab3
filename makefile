CC = $(CROSS_COMPILE)gcc
all:
	$(CC) -pthread -o server server.c
	$(CC) -o client client.c
clean:
	rm -f server client server.o client.o

