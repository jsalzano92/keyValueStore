#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
  KEY VALUE VARIABLES and FUNCTIONS
*/

char* kvStore [1000];


typedef struct header{
        char type;
        int kSize;
        int vSize;
} header;

void putKV(header* h);
void getKV(header* h);
void rmKV(header* h);
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
        if(argc < 2) {
                printf("usage: %s <port number>\n", argv[0]);
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
                        printf("Failed to accept\n");
                pid = fork();
                if(pid == 0) {
                        listenToClient();
                        printf("Client Disconected\n");
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
                printf("Cant open socket!\n");
                return 0;
        }
        bzero((char*) &serv_addr, sizeof(serv_addr));

        portN = port;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portN);

        if(bind(sockt, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                printf("Failed to bind\n");
                return 0;
        }

        return 1;
}

void listenToClient() {
        int len;
        header *head = malloc(sizeof(header));
        while(1) {
                len = recv(client, head, sizeof(header), 0);
                if(len != sizeof(header))
                        break;
                //parseHeader(header, len,);

                if(head->type == 'p') {
                        putKV(head);
                }
                else if(head->type == 'g') {
                        getKV(head);
                }
                else if(head->type == 'r') {
                        rmKV(head);
                }
        }
        free(head);
        close(client);
        exit(0);
}

void putKV(header* h) {
        char* key = malloc(h->kSize);

        if(recv(client, key, h->kSize, 0) != h->kSize) {
                write(client, "badHeadP", 10);
                free(key);
                return;
        }

        write(client, "inPut", 10);

        //Hashmap key!!
        //Check if key is used

        free(key);

        //kvStore[0] = malloc(h->vSize);
        char* test = malloc(h->vSize);

        recv(client, test, h->vSize, 0);

        //TESTS BY SENDING THE RECIEVED DATA RIGHT BACK
        send(client, test, h->vSize, 0);
        //write(client, "stored", 10);
        return;
}

void getKV(header* h) {
        char* key = malloc(h->kSize);

        if(recv(client, key, h->kSize, 0) != h->kSize) {
                write(client, "badHead", 10);
                free(key);
                return;
        }

        write(client, "inGet", 10);

        //Hashmap key!!
        //Check if key is used

        free(key);

        //JUST SEND A TEST!
        if(!kvStore[0])
                kvStore[0] = "This is a test.";

        header *headBack = malloc(sizeof(header));
        headBack->vSize = strlen(kvStore[0]);
        send(client, headBack, sizeof(header), 0);

        send(client, kvStore[0], strlen(kvStore[0]), 0);
        write(client, "shouldGet", 10);
        return;
}


void rmKV(header* h) {
        char* key = malloc(h->kSize);

        if(recv(client, key, h->kSize, 0) != h->kSize) {
                write(client, "badHeadR", 10);
                free(key);
                return;
        }

        free(key);

        write(client, "Removed", 10);

        return;
}
