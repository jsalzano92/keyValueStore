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
int getData(char* buff, int size, FILE* file);
int connectToServer(char* host, int port);
int printBuff(char* buffer, int bufferSize);
int saveFile(char* buff, char* fileName, int size);

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

//HANDLES CONECTION TO SERVER
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
	//Gets the key and filename of what is being sent to server.
    char keyToStore[10]; //<< NOT USED AT MOMENT
    char valueToStore[80];

    printf("Enter Key: ");
    if(!getString(keyToStore, 10))
        return 0;

    printf("Enter Value: ");
    if(!getString(valueToStore, 80))
        return 0;

	header *head = (header*)malloc(sizeof(header));

	//WILL consolidate to the get data function eventually
	FILE* fp = fopen(valueToStore, "rb");	
	if(!fp)
		return 0;

	fseek(fp, 0L, SEEK_END);
	int fSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);		

	//For Testing
	printf("\n::%d::\n", fSize);
	
	char* buff = (char*)malloc(fSize);
	if(!getData(buff, fSize, fp))
		return 0;
	
	//Set up header to send to Server so the server knows whats coming.
	head->type = 'p';
	head->kSize = strlen(keyToStore);
	head->vSize = fSize;

	//Sends the header and then the key.
	send(sockt, head, sizeof(header), 0);
	send(sockt, keyToStore, strlen(keyToStore), 0);
	free(head);	

	//For testing server responce. Old Test
	int test[10];
	read(sockt, test, 10);
	printf("%s\n", test);

	//TestFile
	saveFile(buff, "testBeforeSent", fSize); //<< matches for both text and binary files

	// Tests the size of packet sent as well as sends the packer to Server
	printf("\n%d\n", send(sockt, buff, fSize, 0));

	//Tests what the server received by getting it right back
	char* testbuff = (char*)malloc(fSize);
	printf("\n%d\n", recv(sockt, testbuff, fSize, 0));
	
	//TestFile
	saveFile(testbuff, "testAfterReceived", fSize); //<< does not match inputFile for binary files, works for text

	free(buff);
	free(testbuff);
	return 1;
}

int get() {
    printf("Enter Key: ");
    char keyToCheck[10];
    if(!getString(keyToCheck, 10))
        return 0;

	header *head = (header*)malloc(sizeof(header));
	
	head->type = 'g';
	head->kSize = strlen(keyToCheck);
	head->vSize = 0;

	send(sockt, head, sizeof(header), 0);	
	send(sockt, keyToCheck, strlen(keyToCheck), 0);

	char test[10];
	read(sockt, test, 10);
	printf("%s\n",test);
	
	printf("\n\n%d\n\n", recv(sockt, head, sizeof(header), 0));

	free(head);
	int fSize = head->vSize;
	printf("%d",fSize);	
	char* buff = (char*)malloc(fSize);
	if(recv(sockt, buff, fSize, 0) != fSize) {
		free(buff);
		return 0;
	}

	saveFile(buff, "testRecievedFile", fSize);	

	read(sockt, test, 10);
	printf("%s\n",test);
	
	free(buff);
	return 1;
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


//Read File to Buffer
int getData(char* buff, int size, FILE* file)
{
	//memcpy(buff, file, size); //<< NOT WORKING
	fread(buff, size, 1, file); //<< NOT WORKING FOR BINARY
	printBuff(buff, size);
	return 1;
}


//--------------------------------
//NOT MY CODE, TAKEN FROM INTERNET
//USED FOR TESTING ONLY
//--------------------------------
int printBuff(char* buffer, int bufferSize) {
	for (int c=0;c<bufferSize;c++)
	{
		printf("%.2X ", (int)buffer[c]);
		if (c % 4 == 3)
			printf(" ");
        if (c % 16 == 15)
			printf("\n");
	}
		printf("\n");
}

//Save buffer to File
int saveFile(char* buff, char* fileName, int size) {
	FILE* file = fopen(fileName, "wb");
	fwrite(buff, size, 1, file);
	fclose(file);
}
