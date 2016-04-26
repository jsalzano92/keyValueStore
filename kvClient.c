#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int put();
int get();
int rm();
int getString(char* s, int size); //all buffer handling
int connectToServer(char* host, int port);
int sockt, portN, serverReturn;
struct sockaddr_in serv_addr;
struct hostent* server;
	
typedef struct header {
	char type;
	int	kSize;
	int vSize;
} header;

int main(int argc, char* argv[]) {

	if(!connectToServer(argv[1], atoi(argv[2]))) {
		printf("Could Not Connect To Server.");
		return -1;
	}

	char command[10];
    do {
        printf("Command: ");
    	getString(command, 10);
	    if(strcmp(command, "put") == 0) {
			if(put())
                printf("Value Stored\n");
            else
                printf("Error in storing Value\n");
        }
        else if(strcmp(command, "get") == 0) {
            if(!get())
                printf("Could not locate Value with given Key\n");
        }
        else if(strcmp(command, "rm") == 0) {
            if(!rm())
                printf("Could not remove Value with given Key\n");
            else
                printf("Value Removed\n");
        }
    } while(strcmp(command, "quit") != 0);
	write(sockt, "q", 1);
	return 0;
}

int connectToServer(char* host, int port) {
	portN = port;
	sockt = socket(AF_INET, SOCK_STREAM, 0);
	server = gethostbyname(host);
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portN);
	if(connect(sockt,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
		printf("Cant connect");
		return 0;
	}

	return 1;
}

int put()
{
    char keyToStore[10];
    char valueToStore[80];

    printf("Enter Key: ");
    if(!getString(keyToStore, 10))
        return 0;

    printf("Enter Value: ");
    if(!getString(valueToStore, 80))
        return 0;

	header *head = (header*)malloc(sizeof(header));
	
	head->type = 'p';
	head->kSize = strlen(keyToStore);
	head->vSize = strlen(valueToStore);

	send(sockt, head, sizeof(header), 0);
	
	char test[10];
	read(sockt, test, 10);
	printf("%s\n",test);
	
	free(head);
    return 1;
}

int get() {
    printf("Enter Key: ");
    char keyToCheck[10];
    if(!getString(keyToCheck, 10))
        return 0;

    // GET FROM SERVER
    // char* val = getValue(keyToCheck, kvList);

	char val[80];

	header *head = (header*)malloc(sizeof(header));
	
	head->type = 'g';
	head->kSize = strlen(keyToCheck);
	head->vSize = 0;

	send(sockt, head, sizeof(header), 0);
	
	char test[10];
	read(sockt, test, 10);
	printf("%s\n",test);
	
	free(head);
    return 0;
}

int rm() {
    printf("Enter Key: ");
    char keyToRemove[10];
    if(!getString(keyToRemove, 10))
        return 0;

	header *head = (header*)malloc(sizeof(header));
	
	head->type = 'r';
	head->kSize = strlen(keyToRemove);
	head->vSize = 0;

	send(sockt, head, sizeof(header), 0);

	char test[10];
	read(sockt, test, 10);
	printf("%s\n",test);
	
	free(head);
    return 0;
}

int getString(char* s, int size) {
	char cc; // char to clear buffer
	char* sPoint; // to check my string

	if(fgets(s, size, stdin) != NULL) { //make sure we have something
		if((sPoint = strchr(s, '\n')) != NULL) { //removes trailing newLine char if we did not use full amount of size
			*sPoint = '\0';
			return 1;
		} else { //clears buffer if needed
			while((cc = getchar()) != '\n');
			return 1;
		}
	}
	else
		return 0;
}
