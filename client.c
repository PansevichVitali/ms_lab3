#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

#define BUF_LENGTH 1000

int main()
{
	int sock, r, port;
	long fileSize, sizeCheck = 0;
	struct sockaddr_in addr;
	char ip[100];
	char filename[BUF_LENGTH], fileSizeChar[BUF_LENGTH];
	char content[BUF_LENGTH];
	FILE *fp;

	printf("Enter ip address: ");
	scanf("%s", ip);

	printf("Enter port: ");
	scanf("%d", &port);

	printf("Enter filename: ");
	scanf("%s", filename);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Connect");
		exit(2);
	}

	send(sock, filename, sizeof(filename), 0);
	recv(sock, fileSizeChar, sizeof(fileSizeChar), 0);
	fileSize = atoi(fileSizeChar);
	fp = fopen(filename, "w");

	while (sizeCheck < fileSize) {
		r = recv(sock, content, sizeof(content), 0);
		sizeCheck += r;
		fwrite(content, 1, r, fp);
	}

	printf("File has been sent successfully\n");

	fclose(fp);
	close(sock);

	return 0;
}

