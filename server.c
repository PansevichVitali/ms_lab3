#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

const int readSize = 1024;

struct sockFile
{
  int sock;
  char filename[1024];
};

void* thread_func (void* param)
{
  long fileSize = 0, sizeCheck = 0;
  char content[readSize];
  char fileSizeChar[1024];
  struct sockFile *sf = (struct sockFile*)param;
  FILE *fp;

  fp = fopen(sf->filename, "r");
  if (fp == NULL) {
    printf("Unable to open '%s'\n", sf->filename);
    exit(1);
  }

  fseek (fp, 0 , SEEK_END);

  fileSize = ftell (fp);
  rewind (fp);

  snprintf(fileSizeChar, sizeof(fileSizeChar), "%ld", fileSize);
  send(sf->sock, fileSizeChar, 1024, 0);

  while (sizeCheck < fileSize){
    int read, sent;
    read = fread(content, 1, readSize, fp);
    sent = send(sf->sock, content, read, 0);
    sizeCheck += sent;
  }

  printf("File '%s' has been received successfully\n", sf->filename);

  close(sf->sock);
  free(sf);
  fclose(fp);
}

int main(int argc, char *argv[]) 
{
  int port = 1234;         // default port number to use
  int sock, listener, rc;
  struct sockaddr_in addr;
  char filename[1024];
  int bytes_read;

  // check command line arguments, handling an optional port number
  if ((argc == 2) || (argc == 3)) {
    port = atoi(argv[1]);
    if (port <= 0) {
      printf("Invalid port: %s\n", argv[1]);
      exit(1);
    } 
  } else if (argc != 1) {
    printf("Usage: %s [port] [p|t]\n", argv[0]);
    exit(1);
  }

  // reate Internet domain socket
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
      struct sockFile *sf;
      pthread_t thread_id;

      // wait for a client to connect
      sock = accept(listener, NULL, NULL);
      sf = (struct sockFile*) malloc (sizeof(struct sockFile));

      if(sock < 0) {
          perror("Accept");
          exit(3);
      }
      bytes_read = recv(sock, filename, 1024, 0);
      sf->sock = sock;

      strcpy(sf->filename, filename);

      if (argv[2] == "t") {
	rc = pthread_create(&thread_id, NULL, thread_func, (void*)sf);
      	if (rc)
            printf("Can't create thread!");
      }
      else {
	pid = fork();
        if (pid == 0) thread_func((void*)sf);
      }
  }
  return 0;
}