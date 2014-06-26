CC=$(CROSS_COMPILE)gcc
all:
	$(CC) client.c -o client
clean:
	rm -rf client
