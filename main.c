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
#include "blowfish.h"
#include <time.h>
#define PORT 776

static SOCKET sock, csock;

void close_sock() {
	printf("Quit\n");
	shutdown(csock, 2);
	close(sock);
	close(csock);
}

void send_data(const char *data, const unsigned long size, const char *content_type) {
	char send_buff[500+size];
	unsigned long new_size = size;
	if(make_response(data, &new_size, send_buff, content_type) == 0) {
		printf("Send buff : %s", send_buff);
		if(send(csock, send_buff, new_size, 0) == SOCKET_ERROR)
			printf("Sending error\n");
	}
}

int authentication() { // Return value : 1 = Error ; -1 = No error but wrong password ; 0 = everything's good
	// 1. Generate password
	char password[PASSWORD_LEN];
	generate_password(password);
	
	// 2. Write the password in the index.html file
	FILE* index_html = fopen(IMG_DIRECTORY "/../index.html", "r+");
	if (index_html == NULL) {
		perror("fopen");
		printf("File requested : %s\n", IMG_DIRECTORY "/../index.html");
		return 1;
	}
	int i;
	for (i = 1 ; i < INDEX_HTML_PASSWORD_LINE ; i++) {
		char c;
		while ((c = fgetc(index_html)) != '\n') {
			if (c == EOF)
				return 1;
		}
	}
	fprintf(index_html, "var password = \"%s\";", password);
	fclose(index_html);
	
	// 3. Send data
	char *data = NULL;
	unsigned long size = 0;
	
	char content_type[200] = "";
	if(get_file("/../index.html", &data, &size, content_type) == 1)
		return 1;
	
	send_data(data, size, content_type);
	free(data);
	
	// 4. Wait for answer
	char *crypt_answer = NULL;
	
	while (1) { // There may be many requests before sending the password (at least two : 1. Blowfish.js, 2. Password)
		int r = 0;
		char recv_buff[500] = "";
		if((r = recv(csock, recv_buff, 500, 0)) == SOCKET_ERROR) {
			printf("Reception error while authenticating\n");
			return 1;
		}
		else if(r == 0) {
			printf("Client quit while authenticating\n");
			shutdown(csock, 2);
			close(csock);
			return 1;
		}
		
		//printf("Recv buff: %s\n", recv_buff);
		
		strtok(recv_buff, " ");
		if (strcmp(crypt_answer = strtok(NULL, " "), "/Blowfish.js") == 0) {
			if(get_file("/../blowfish/Blowfish.js", &data, &size, content_type) == 1)
				return 1;
			send_data(data, size, content_type);
			free(data);
		}
		else if (crypt_answer[0] != '/') {
			continue;
		}
		else
			break;
	}
	
	// 5. Get the encrypted answer
	if(crypt_answer[strlen(crypt_answer)-1] == '/') crypt_answer[strlen(crypt_answer)-1] = '\0'; // Remove the last /
	crypt_answer++; // Remove the first /
	
	// 6. Get the blowfish key and generate subkeys
	FILE* key_file = fopen(IMG_DIRECTORY "/../key", "r");
	if (key_file == NULL) {
		perror("fopen");
		printf("File requested : %s\n", IMG_DIRECTORY "/../key");
		return 1;
	}
	char key[500] = "";
	if(fgets(key, 500, key_file) == NULL)
		return 1;
	fclose(key_file);
	
	*strchr(key, '\n') = '\0';
	
	// 7. Verify answer by decrypting it
	char decrypt_answer[PASSWORD_LEN*2] = "";
	decrypt(decrypt_answer, key, crypt_answer);
	printf("Decrypted answer: %s\n", decrypt_answer);
	if (strcmp(decrypt_answer, password) == 0)
		return 0;
	else
		return -1;
}
 
int main(int argc, char* argv[]) {
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
	
	
	char last_client[100] = "";
	time_t last_time = time(NULL);
	
waiting_for_client:
	csock = accept(sock, (SOCKADDR*)&csin, &crecsize);
	printf("Connexion from %s with socket %d\n", inet_ntoa(csin.sin_addr), csock);
	
	while(1) {
		char recv_buff[500] = "";
		
		int r = 0;
		if((r = recv(csock, recv_buff, 500, 0)) == SOCKET_ERROR) {
			printf("Reception error\n");
			continue;
		}
		else if(r == 0) {
			printf("Client quit\n");
			shutdown(csock, 2);
			close(csock);
			goto waiting_for_client;
		}
		
		if(strnlen(recv_buff, 500) == 500) { // To avoid some segment errors
			printf("Reception error\n");
			continue;
		}
		printf("Received : \n%s\n", recv_buff);
	
		strtok(recv_buff, " ");
		
		char *directory = strtok(NULL, " ");
		
		if (strcmp(last_client, inet_ntoa(csin.sin_addr)) != 0 || time(NULL) > last_time + 60*5) { // The client is new OR the actual client is inactive since more than 5 minutes : Ask for authentication
			do {
				r = authentication();
				if (r == 1) { // Error
					printf("Authentication error.\n");
					goto waiting_for_client;
				}
				else if (r == -1) // Bad password
					printf("Raaah ! Authentication failed !!!!!!\n");
				else if (r == 0) { // Authentication successful
					printf("Authentication successful\n");
					directory[0] = '\0';
					break;
				}
			} while (1);
			strcpy(last_client, inet_ntoa(csin.sin_addr));
		}
		
		last_time = time(NULL); // Update last_time
		
		
		if(directory[strlen(directory)-1] == '/') directory[strlen(directory)-1] = '\0'; // There will be problems if we don't remove the last /.
		printf("File or directory requested: %s\n", directory);
			
		// Get data to send
		char *data = NULL;
		char content_type[200] = "";
		unsigned long size = 0;
		
		if(strstr(directory, "/."))
			get_unauthorized(&data, &size, content_type, argv[0]);
		else if(get_dir(directory, &data, &size, content_type) == 1) {
			if(get_file(directory, &data, &size, content_type) == 1)
				get_404(&data, &size, content_type, argv[0]);
		}
		
		send_data(data, size, content_type);
		free(data);
	}
		
	return EXIT_SUCCESS;
}

