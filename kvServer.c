#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
	KEY VALUE VARIABLES and FUNCTIONS
*/

typedef struct header{
	char type;
	int kSize;
	int vSize;
} header;

int putKV(char* key, char* ); //to associate value with a key
int getKV(); //to get a value with a key
int rmKV();
/*
	END KEY VALUE VARIABLES and FUNCTIONS

	=========================================

	SERVER VARIABLES and FUNCTIONS
*/

int sockt;
int client;
int portN;
int clientReturn;
int pid;
socklen_t clientLen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;

int initServer(int port);
void listenToClient();

/*
	END SERVER VARIABLES and FUNCTIONS
	
	=========================================

	ADMIN VARIABLES and FUNCTIONS
*/

char* password = "ymca123abc";
int adminMode = 0;
void shutDownServer();
void clearDatabase();

/*
	END ADMIN VARIABLES and FUNCTIONS
*/

int main(int argc, char* argv[]) {
	//get port from user
	if(argc != 2) {
		printf("Port Number Needed");
		return 1;
	}
	//Start Server
	if(!initServer(atoi(argv[1])))
		return -1;
	listen(sockt, 5);	
	clientLen = sizeof(cli_addr);
		
	while(1) {
		client = accept(sockt, (struct sockaddr*) &cli_addr, &clientLen);
		if(client < 0)
			printf("Failed to accept");
		pid = fork();
		if(pid == 0) {
			listenToClient();
			printf("Client Disconected");
			exit(0);
		} else {
			close(client);
		}
	}
	close(sockt);
	return 0;
}

int initServer(int port) {
	sockt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockt < 0) {
		printf("Cant open socket!");
		return 0;
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));

	portN = port;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portN);
	
	if(bind(sockt, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Failed to bind");
		return 0;
	}

	return 1;
}

void listenToClient() {
	int len;
	header *head = (header*)malloc(sizeof(header));
	while(1) {
		len = recv(client, head, sizeof(header), 0);
		if(len != sizeof(header))
			break;
		//parseHeader(header, len,);

		if(head->type == 'p') {
			write(client, "put", 10);
		}
		else if(head->type == 'g') {
			write(client, "get", 10);
		}
		else if(head->type == 'r') {
			write(client, "remove", 10);
		}
	}
	free(head);
	close(client);
	exit(0);
}

