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
/*
	END SERVER VARIABLES and FUNCTIONS
*/

int main(int argc, char* argv[]) {
	printf("This is the start");
	//get port from user
	if(argc != 2) {
		printf("Port Number Needed");
		return 1;
	}
	printf("Whill it even get here?");
	//Start Server
	initServer(atoi(argv[1]));
	//	return 1;
	printf("Testing");
	listen(sockt, 5);
	clientLen = sizeof(cli_addr);
		
	while(1) {
		printf("waiting for connection");
		client = accept(sockt, (struct sockaddr*) &cli_addr, &clientLen);
		if(client < 0)
			printf("Failed to accept");
		pid = fork();
		if(pid == 0) {
			printf("connected");
			listenToClient();
			exit(0);
		} else {
			close(client);
		}
	}
	close(sockt);
	return 0;
}

int initServer(int port) {
	printf("here?");
	sockt = socket(AF_INET, SOCK_STREAM, 0);
	if(sockt < 0) {
		printf("Cant open socket!");
		return 0;
	}
	printf("Trying..");
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
	printf("Listening");
	char header[10];
	char type;
	int	size;	
	do {
		read(client, header, 10);
		//printf("got something");
		//fgets(type,sizeof(char),header);
		if(header[0] == 'p') {
			write(client, "Going to Put", 10);
		}
		else if(header[0] == 'g') {
			write(client, "Going to Get", 10);
		}
		else if(header[0] = 'r') {
			write(client, "Going to Remove", 10);
		}
	} while(strcmp(header, "quit") != 0);
	close(client);
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
