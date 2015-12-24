#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "get_dir.h"
#define PORT 776

static SOCKET sock, csock;

void close_sock() {
	shutdown(csock, 2);
	close(sock);
	close(csock);
}

void send_data(const char *data, const unsigned long size, const char *content_type) {
	char send_buff[500+size];
	if(make_response(data, size, send_buff, content_type) == 0) {
		printf("Send buff : %s", send_buff);
		if(send(csock, send_buff, size+strlen(send_buff), 0) == SOCKET_ERROR)
			printf("Sending error\n");
	}
}
 
int main() {
	atexit(close_sock);
	
	SOCKADDR_IN sin;
	socklen_t recsize = sizeof(sin);
	
	SOCKADDR_IN csin;
	socklen_t crecsize = sizeof(csin);
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sock == INVALID_SOCKET) {
		perror("socket");
		close(sock);
		return EXIT_FAILURE;
	}
	
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	
	if(bind(sock, (SOCKADDR*)&sin, recsize) == SOCKET_ERROR) {
		perror("bind");
		close(sock);
		return EXIT_FAILURE;
	}
	
	if(listen(sock, 5) == SOCKET_ERROR) {
		perror("listen");
		close(sock);
		return EXIT_FAILURE;
	}
	
	while(1) {
		csock = accept(sock, (SOCKADDR*)&csin, &crecsize);
		printf("Connexion from %s with socket %d\n", inet_ntoa(csin.sin_addr), csock);

			char recv_buff[500] = "";
	
			if(recv(csock, recv_buff, 500, 0) == SOCKET_ERROR)
				printf("Reception error\n");
		
			printf("Received : \n%s\n", recv_buff);
		
			strtok(recv_buff, " ");
		
			char *directory = strtok(NULL, " ");
			if(directory[strlen(directory)-1] == '/') directory[strlen(directory)-1] = '\0'; // There will be problems if we don't remove the last /.
			printf("File or directory requested: %s\n", directory);
			
			// Get data to send
			char *data = NULL;
			char content_type[200] = "";
			unsigned long size = 0;
			
			if(strstr(directory, "/."))
				get_unauthorized(&data, &size, content_type);
			else if(get_dir(directory, &data, &size, content_type) == 1) {
				if(get_file(directory, &data, &size, content_type) == 1)
					get_404(&data, &size, content_type);
			}
			send_data(data, size, content_type);
			free(data);
		
		shutdown(csock, 2);
		close(csock);
	}
		
	return EXIT_SUCCESS;
}