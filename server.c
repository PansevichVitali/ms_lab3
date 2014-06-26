#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

const int readSize = 1024;

struct sockFile
{
  int sock;
  char filename[1024];
};

void* ThreadFunc (void* param)
{
  long fileSize = 0, sizeCheck = 0;
  char content[readSize];
  char fileSizeChar[1024];
  struct sockFile *sf = (struct sockFile*)param;
  FILE *fp;

  fp = fopen(sf->filename, "r");
  fseek (fp, 0 , SEEK_END);

  fileSize = ftell (fp);
  rewind (fp);

  snprintf(fileSizeChar, sizeof(fileSizeChar), "%ld", fileSize);
  send(sf->sock, fileSizeChar, 1024, 0);

  while (sizeCheck < fileSize)
  {
    int read, sent;
    read = fread(content, 1, readSize, fp);
    sent = send(sf->sock, content, read, 0);
    sizeCheck += sent;
  }

  close(sf->sock);
  free(sf);
  fclose(fp);
}

int main(int argc, char *argv[]) {
  int sock, listener, rc;
  struct sockaddr_in addr;
  char filename[1024];
  int bytes_read;

  listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
      perror("socket");
      exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3425);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("bind");
      exit(2);
  }

  listen(listener, 1);

  while (1) {
      int pid;
      struct sockFile *sf;
      pthread_t thread_id;

      sock = accept(listener, NULL, NULL);
      sf = (struct sockFile*) malloc (sizeof(struct sockFile));

      if(sock < 0) {
          perror("accept");
          exit(3);
      }
      bytes_read = recv(sock, filename, 1024, 0);
      sf[0].sock = sock;

      strcpy(sf[0].filename, filename);

      if (argv[1] == "t") {
	rc = pthread_create(&thread_id, NULL, ThreadFunc, (void*)&sf[0]);
      	if (rc)
            printf("Can't create thread!");
      }
      else {
	pid = fork();
        if (pid == 0) ThreadFunc((void*)&sf[0]);
      }
  }
  return 0;
}