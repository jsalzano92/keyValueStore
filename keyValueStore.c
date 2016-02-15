#include <stdio.h>
#include <string.h>

char* KEY[10];
char* VALUE[10];
int numberStored = 0;

int put();

int main(int argc, char* argv[]){

	char command[10];
	do {
		printf("Command: ");
		fgets(command, 10, stdin);
		if(strcmp(command, "put\n") == 0) {
			if(put())
				printf("Value Stored\n");
			else
				printf("Error in Storing\n");				
		}
		if(strcmp(command, "get\n") == 0) {
			for(int i = 0; i < 10; i++)
				printf("KEY: %s\tVALUE: %s\n", KEY[i], VALUE[i]);
		}
	} while(strcmp(command, "quit\n") != 0);
	
	return 0;
}

int put()
{
	if(numberStored >= 10) {
		printf("No more room");
		return 0;
	}

	char keyToStore[10];
	char valueToStore[80];	

	printf("Enter Key, under 10 characters and must be unique: ");	
	fgets(keyToStore, 10, stdin);
	keyToStore[strcspn(keyToStore, "\n" )] = '\0';
	puts(keyToStore);
	printf("Enter String to store (max 80 characters): ");
	fgets(valueToStore, 80, stdin);	
	valueToStore[strcspn(valueToStore, "\n")] = '\0';
	puts(valueToStore);
	
	KEY[numberStored] = keyToStore;
	VALUE[numberStored] = valueToStore;

    printf("KEY: %s\tVALUE: %s\n", KEY[numberStored], VALUE[numberStored]);
	
	numberStored++;

	return 1;
}
