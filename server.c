#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LENGTH 1000
#define PTHREAD

struct sock_file
{
	int sock;
	char filename[BUF_LENGTH];
};

void* thread_func (void* param)
{
	long file_size = 0, size_check = 0;
	char content[BUF_LENGTH];
	char file_size_char[BUF_LENGTH];
	struct sock_file *sf = (struct sock_file*)param;
	FILE *fp;

	fp = fopen(sf->filename, "r");
	if (fp == NULL) {
		printf("Unable to open '%s'\n", sf->filename);
		exit(1);
	}

	fseek(fp, 0 , SEEK_END);

	file_size = ftell(fp);
	rewind(fp);

	snprintf(file_size_char, sizeof(file_size_char), "%ld", file_size);
	send(sf->sock, file_size_char, BUF_LENGTH, 0);

	while (size_check < file_size){
		int read, sent;
		read = fread(content, 1, BUF_LENGTH, fp);
		sent = send(sf->sock, content, read, 0);
		size_check += sent;
	}

	printf("File '%s' has been received successfully\n", sf->filename);

	close(sf->sock);
	free(sf);
	fclose(fp);
}

int main(int argc, char *argv[])
{
	int port = 1111;         // default port number to use
	int sock, listener, rc;
	struct sockaddr_in addr;
	char filename[BUF_LENGTH];
	int bytes_read;

	// create Internet domain socket
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0) {
		perror("Socket");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		exit(2);
	}

	listen(listener, 1);

	int pid;
	while (1) {
		struct sock_file *sf;
		pthread_t thread_id;

		// wait for a client to connect
		sock = accept(listener, NULL, NULL);
		sf = (struct sock_file*) malloc (sizeof(struct sock_file));

		if (sock < 0) {
			perror("Accept");
			exit(3);
		}
		bytes_read = recv(sock, filename, BUF_LENGTH, 0);
		sf->sock = sock;

		strcpy(sf->filename, filename);
#ifdef PTHREAD
		rc = pthread_create(&thread_id, NULL, thread_func, (void*)sf);
		if (rc)
			printf("Can't create thread!");
#else
		pid = fork();
		if (pid == 0) thread_func((void*)sf);
		}
#endif
	}
	return 0;
}
