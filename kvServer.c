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
typedef struct link {
	char key[10];
	char value[80];
	struct link *next;
} keyVal;

typedef struct header{
	char type;
	int kSize;
	int vSize;
} header;

keyVal* kvList;

int put(); //to associate value with a key
int get(); //to get a value with a key
int rm();
int getString(char* s, int size); //all buffer handling
keyVal* createKV(char* k, char* v); // create a new node
int checkKey(char* key, keyVal* list); //checking if key is in array KEY
char* getValue(char* key, keyVal* list);
int rmValue(char* key, keyVal* list, keyVal* prev);
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
void promptClient(char* message);
void listenToClient();
void parseHeader(char *header, int len, char *type, int *size0, int *size1);

/*
	END SERVER VARIABLES and FUNCTIONS
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



void promptClient(char* message) {
	if(write(client, message, strlen(message)) < 0)
		printf("Error writing message: %s to client\n", message);	
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

void parseHeader(char *header, int len, char *type, int *size0, int *size1) {
	
}

/*
	Linked List Functions
*/

keyVal* createKV(char k[], char v[]) {
	keyVal *toReturn = (keyVal*)malloc(sizeof(keyVal));
	strcpy(toReturn->key, k);
	strcpy(toReturn->value, v);
	toReturn->next = kvList;
	return toReturn;
}

int checkKey(char* key, keyVal* list)	{
	if(list == NULL)
		return 0;
	
	if(strcmp(list->key, key) == 0)
		return 1;
	else
		return checkKey(key, list->next);
}

char* getValue(char* key, keyVal* list) {
	if(list == NULL)
		return NULL;

	if(strcmp(list->key, key) == 0)
		return list->value;
	else
		return getValue(key, list->next);
}

int rmValue(char* key, keyVal* list, keyVal* prev) {
	if(list == NULL)
		return 0;

	if(strcmp(list->key, key) == 0) {
		if(prev == NULL)
			kvList = list->next;
		else
			prev->next = list->next;
		free(list);
		return 1;
	}
	else {
		return rmValue(key, list->next, list);
	}
}
