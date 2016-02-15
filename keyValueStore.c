#include <stdio.h>
#include <string.h>

char KEY[10][10]; //Array of 10 Strings each with a max size of 10 for holding the key
char VALUE[10][80];	//Array of 10 Strings each with a max size of 80 for holding the value
int numberStored = 0; // int to keep track of number of stored key-values

int put(); //to associate value with a key
int get(); //to get a value with a key
int checkKey(char* key); //checking if key is in array KEY
int getString(char* s, int size); //all buffer handling

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
			if(!get())
				printf("Could Not Locate Value with given Key\n");
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
	if(!getString(keyToStore, 10))
		return 0;
	if(checkKey(keyToStore)) {
		printf("Key already in use\n");
		return 0;
	}
	printf("Enter String to store (max 80 characters): ");
	if(!getString(valueToStore, 80))
		return 0;

	strcpy(KEY[numberStored], keyToStore);
	strcpy(VALUE[numberStored], valueToStore);

	numberStored++;

	return 1;
}


int get() {
	printf("Enter Key: ");
	char keyToCheck[10];
	if(!getString(keyToCheck, 10))
		return 0;

	for(int i = 0; i < numberStored; i++) {
		if(strcmp(KEY[i], keyToCheck) == 0) {
			printf("%s\n", VALUE[i]);
			return 1;
		}
	}

	return 0;
}

int checkKey(char* key) {
	for(int i = 0; i < numberStored; i++)
		if(strcmp(KEY[i], key) == 0)
			return 1;
	
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
