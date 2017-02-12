#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "kvStore.h"


//KEY VALUE VARIABLES and FUNCTIONS
typedef struct header{
        char type;
        int kSize;
        int vSize;
} header;

typedef struct client_struct{
        int client;
} client_struct;

void putKV(header* h, int client);
void getKV(header* h, int client);
void rmKV(header* h, int client);

//SERVER VARIABLES and FUNCTIONS
int sockt;
int portN;
int clientReturn;
int pid;
socklen_t clientLen;
struct sockaddr_in serv_addr, cli_addr;
int initServer(int port);
int initDatabase();
void* listenToClient(void* c);

//ADMIN VARIABLES and FUNCTIONS
char* password = "ymca123abc";
int accessAdmin(header* h, int client);
void shutDownServer();

int main(int argc, char* argv[]) {
        //get port from user
        if(argc < 2) {
                printf("usage: %s <port number>\n", argv[0]);
                return 1;
        }
        
        //Start Server
        if(!initServer(atoi(argv[1])))
                return -1;
        //Create Key Value Storage
        if(!initDatabase())
                return -1;

        //Wait for client connections
        listen(sockt, 5);
        clientLen = sizeof(cli_addr);

        //Handle client connections
        while(1) {
                int client = accept(sockt, (struct sockaddr*) &cli_addr, &clientLen);
                if(client < 0)
                        printf("Failed to accept\n");
                
                client_struct* clientInfo = malloc(sizeof(client_struct));
                clientInfo->client = client;
                
                pthread_t threadID;
                pthread_create(&threadID, NULL, &listenToClient, clientInfo);
                printf("Thread Created With Client ID: %d and Thread ID: %lu\n", client, threadID);
        }
        shutDownServer();
        return 0;
}

//Function creates server side. Still don't understand the proccess completely, more research is needed. This came from a UNIX networking tutorial online. Customized slightly.
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

//Function to listen to client for headers. When a header is received it finds what type of request the client sent and sends the header to the correct function to handle the request.
void* listenToClient(void* c) {
        int admin = 0;
        
        client_struct* clientInfo = (client_struct*) c;
        int client = clientInfo->client;
        free(clientInfo);
        
        //Loops untill the client sends a packet that is not expected then breaks the connection.
        //This is also forced on the client side when the command is quit, the client purposly sends a single character buffer.
        while(1) {
                header *head = malloc(sizeof(header));
                int len = recv(client, head, sizeof(header), 0);
                if(len != sizeof(header)){ //Breaks connection when unexpected packet is received.
                        free(head);
                        break;
                }

                if(head->type == 'p') {
                        putKV(head, client);
                }
                else if(head->type == 'g') {
                        getKV(head, client);
                }
                else if(head->type == 'r') {
                        rmKV(head, client);
                }
                else if(head->type =='a') {
                        admin = accessAdmin(head, client);
                }
                else if(head->type == 'k') {
                        if(admin){
                                shutDownServer();
                                write(client, "S", 1);
                                exit(0);
                        } else
                                write(client, "F", 1);
                }
                free(head);
        }
        
        close(client);
        pthread_detach(pthread_self());
        return 0;
}

//Receives a data buffer from the client and then calls a function to store the data buffer in our kvStorage system.
void putKV(header* h, int client) {
        //Allocates memory for the key that is expected from the client.
        //Is sent the size in the header that was passed from client.
        //Then reads the key that client sends into memory on the server side.
        char* key = malloc(h->kSize);
        if(recv(client, key, h->kSize, 0) != h->kSize) {
                free(key);
                return;
        }

        //Allocates memory for the data buffer that the client is sending next.
        //Again this information is sent by the client in the header.
        char* toStore = malloc(h->vSize);

        //Loops to receive all the buffer that has been sent by the client.
        //When the expected data is received the loop breaks.
        int received = 0;
        do{
                received += recv(client, &toStore[received], h->vSize-received, 0);
        }while(received < h->vSize);

        //Calls the storage part of the server to store the data buffer as the value that the key points to.
        //Then writes the client the success or failer of the storage function.
        if(addValue(key, toStore, received))
                write(client, "S", 1); // Success
        else
                write(client, "F", 1); // Fail

        return;
}

//Sends the client the data associated with the key that is received.
void getKV(header* h, int client) {
        //Allocates memory for the key that is expected from the client.
        //Is sent the size in the header that was passed from client.
        //Then reads the key that client sends into memory on the server side.
        char* key = malloc(h->kSize);
        if(recv(client, key, h->kSize, 0) != h->kSize) {
                write(client, "badHead", 10);
                free(key);
                return;
        }

        //Allocates the header that is going to be sent back to the client.
        header *headBack = calloc(1, sizeof(header));
        
        //Gets the data buffer associated with the key. And stores the size in vSize.
        int vSize = 0;
        char* toSend = getValue(key, &vSize);
        headBack->vSize = vSize;

        //Sends header to the client to prepare the client for the buffer that will be sent.
        send(client, headBack, sizeof(header), 0);
        
        //Housekeeping
        free(key);
        free(headBack);

        //If the value does not exist for the key received dont send an empty buffer/.
        if(vSize == 0)
                return;
       
        //Send the data to the client. 
        send(client, toSend, vSize, 0);
        
        return;
}

//Function right now just clears the DB to free the memory.
//Not a working function other than that at the moment.
void rmKV(header* h, int client) {
        char* key = malloc(h->kSize);

        if(recv(client, key, h->kSize, 0) != h->kSize) {
                write(client, "badHeadR", 10);
                free(key);
        }
        free(key);

        //Clears then restarts the database for testing purposes.
        clearDB();
        initDatabase();

        write(client, "Removed", 10);

        return;
}

int accessAdmin(header* h, int client) {
        char pw[20];
        if(recv(client, pw, 20, 0) != 20) {
                write(client, "F", 1);
                return 0;
        }
        if(strcmp(pw, password) == 0) {
                write(client, "S", 1);
                return 1;
        }
        else {
                write(client, "F", 1);
                return 0;
        }
}

void shutDownServer() {
        clearDB();
        close(sockt);
}
