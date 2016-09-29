#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int put();
int get();
int rm();
int getString(char* s, int size); //all buffer handling
char* getData(int* size, char* fileName);
int connectToServer(char* host, int port);
void printBuff(char* buffer, int bufferSize);
void saveFile(char* buff, char* fileName, int size);

int sockt, portN, serverReturn;
struct sockaddr_in serv_addr;
struct hostent* server;

typedef struct header {
        char type;
        int  kSize;
        int  vSize;
} header;

int main(int argc, char* argv[]) {
        //Check for arguments
        if(argc < 3) {
                printf("usage: %s <host> <port number>\n", argv[0]);
                return 1;
        }
        //Try to connect to server, sending the host number and the port number to attemp
        //If attemp fails then exit right away.
        if(!connectToServer(argv[1], atoi(argv[2]))) {
                printf("Could Not Connect To Server.\n");
                return -1;
        }

        /*
        Wait for a valad command from terminal.
        When a valad command is received call the neede function.
        All functions called are tested for success and will output message.
        After 20 failed comands or the quit command the program will exit.
        */
        char command[10];
        int failed_lines = 0;
        do {
                printf("Command: ");
                getString(command, 10);
                printf("You entered: %s\n", command);

                if(strcmp(command, "put") == 0) {
                        if(!put())
                                printf("Error in storing Value\n");
                        else
                                printf("Value Stored\n");
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
                else if(strcmp(command, "quit") == 0) {
                        printf("Goodbye!\n");
                }
                else {
                        printf("valid commands include: put, get, rm, and quit\n");
                        failed_lines++;
                        if(failed_lines > 20)
                                strncpy("quit", command, 9);
                }
        } while(strcmp(command, "quit") != 0);
        
        //When the program is set to quit we send a message to 
        //the server connection to let the server know we are exiting.
        write(sockt, "q", 1);
        return 0;
}

//HANDLES CONECTION TO SERVER
//The majority of this function I don't understand completely yet
//Taken from the Linux Ports tutorial website.
int connectToServer(char* host, int port) {
        portN = port;
        sockt = socket(AF_INET, SOCK_STREAM, 0);
        server = gethostbyname(host);
        bzero((char*)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;

        bcopy((char*)server->h_addr_list[0],
              (char*)&serv_addr.sin_addr.s_addr,
              server->h_length);

        serv_addr.sin_port = htons(portN);
        if(connect(sockt,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
                printf("Can't connect\n");
                return 0;
        }

        return 1;
}

//Function to take the key and path to the file to be stored and sends it to the server
int put()
{
        //Gets the key and filename of what is being sent to server.
        char keyToStore[10];
        char valueToStore[80];

        printf("Enter Key: ");
        if(!getString(keyToStore, 10))
                return 0;

        printf("Enter Value: ");
        if(!getString(valueToStore, 80))
                return 0;

        //initializes size variable and reads in file as as string of bytes and stores pointer to data in "buff"
        int fSize;
        char* buff = getData(&fSize, valueToStore);

        //Set up header to send to Server so the server knows whats coming.
        header *head = calloc(1,sizeof(header)); // Allocates memory for header
        head->type = 'p'; // p for put
        head->kSize = strlen(keyToStore) + 1; // + 1 so there is room for \0
        head->vSize = fSize; // Size of buffer to send

        //Sends the header and then the key.
        send(sockt, head, sizeof(header), 0);
        send(sockt, keyToStore, strlen(keyToStore) + 1, 0);
        free(head);

        //Sends main buffer of file data as bytes to the server.
        send(sockt, buff, fSize, 0);
        free(buff);
        
        //Checks for success or failure of the storage.
        //If failure will specify what error before exiting function.
        char result[1];
        read(sockt, result, 1);
        if(result[0] == 'F'){
                printf("Duplicate Key\n");
                return 0;
        } else {
                return 1;
        }
}

//Function to take a key to send to server and save the returned data buffer to a file.
int get() {
        //Gets key to send to server to return value
        printf("Enter Key: ");
        char keyToCheck[10];
        if(!getString(keyToCheck, 10))
                return 0;

        //Sets up header to server to let it know the length of the key and the type of server proccess to run.
        header *head = calloc(1,sizeof(header));
        head->type = 'g';
        head->kSize = strlen(keyToCheck) + 1;
        head->vSize = 0; // no value being sent to server so initialized to 0

        //Sends the header and then the key.
        send(sockt, head, sizeof(header), 0);
        send(sockt, keyToCheck, strlen(keyToCheck) + 1, 0);
        
        //Receives a header sized buffer that overwrites existing allocated space.
        recv(sockt, head, sizeof(header), 0);
        int fSize = head->vSize; // Stores the expected buffer size that is being sent by the server to fSize
        free(head);

        //If there is no buffer coming exit the function and let the main function know that there was an error.
        if(fSize == 0)
                return 0;

        //Allocate memory for the buffer
        char* buff = malloc(fSize);
        //Initialize an int that keeps track of how many bytes have been received from the server
        int received = 0;
        do{
                //Updates how many bytes have been received and stores the ammount received in the buffer
                received += recv(sockt, &buff[received], fSize-received, 0);
        }while(received < fSize); // Continues to read from server untill all the bytes expected are stored into the buffer.

        //Gets the name of the file that the buffer will be saved to.
        char fileToSave[80];
        printf("Enter name and path of file to be saved: ");
        if(!getString(fileToSave, 80))
                return 0;

        //Saves the buffer to the file with the specified name.
        saveFile(buff, fileToSave, fSize);

        //Housekeeping
        free(buff);
        return 1;
}

// RM is under constuction but uses very simular function calls as PUT and GET.
int rm() {
        printf("Enter Key: ");
        char keyToRemove[10];
        if(!getString(keyToRemove, 10))
                return 0;

        header *head = calloc(1,sizeof(header));

        head->type = 'r';
        head->kSize = strlen(keyToRemove);
        head->vSize = 0;

        send(sockt, head, sizeof(header), 0);
        send(sockt, keyToRemove, strlen(keyToRemove), 0);

        free(head);

        char test[10];
        read(sockt, test, 10);
        printf("%s\n",test);

        return 1;
}

// Safest way I found to get a string. Multiple sources from stack overflow mostly
int getString(char* s, int size) {
        char cc; // char to clear buffer
        char* sPoint; // to check my string

        if(fgets(s, size, stdin) != NULL){
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


//Read File to Buffer
char* getData(int* size, char* fileName)
{
        //Opens file for binary reading
        FILE* fp = fopen(fileName, "rb");
        if(!fp) {
                printf("Failed to open file for reading: %s\n", fileName);
                return 0;
        }

        //Calculates the size of the file by seeking to the end and then saving the amount of bytes passed.
        fseek(fp, 0L, SEEK_END);
        *size = ftell(fp);
        //Seeks back to the begining so we can read to buffer.
        fseek(fp, 0L, SEEK_SET);

        //For Testing
        printf("client::put size of file: %d\n", *size);

        //Allocates a data buffer the size of the file.
        char* toReturn = malloc(*size);        

        //Reads the bytes that make up the file to the buffer and closes the file. Then returns a pointer to the data buffer.
        fread(toReturn, *size, 1, fp);
        fclose(fp);
        return toReturn;
}

//Save buffer to File
void saveFile(char* buff, char* fileName, int size) {
        //Opens file for binary writing.
        FILE* file = fopen(fileName, "wb");
        //Writes the data buffer to the specified file.
        fwrite(buff, size, 1, file);
        fclose(file);
}

//--------------------------------
//NOT MY CODE, TAKEN FROM INTERNET
//USED FOR TESTING ONLY
//--------------------------------
void printBuff(char* buffer, int bufferSize) {
        printf("client::printBuff ");
        for (int c = 0; c < bufferSize; ++c) {
                printf("%.2X ", (int)buffer[c]);
                if (c % 4 == 3)
                        printf(" ");
                if (c % 16 == 15)
                        printf("\n");
	}
        printf("\n");
}

