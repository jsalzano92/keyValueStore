#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]) {
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
	
	return 0;
}


int put()
{
	char keyToStore[10];
	char valueToStore[80];	

	printf("Enter Key: ");	
	if(!getString(keyToStore, 10))
		return 0;
	if(checkKey(keyToStore, kvList)) {
		printf("Key already in use\n");
		return 0;
	}
	printf("Enter Value: ");
	if(!getString(valueToStore, 80))
		return 0;

	kvList = createKV(keyToStore, valueToStore);

	return 1;
}


int get() {
	printf("Enter Key: ");
	char keyToCheck[10];
	if(!getString(keyToCheck, 10))
		return 0;

	char* val = getValue(keyToCheck, kvList);
	
	if(val != NULL) {
		printf("%s\n", val);
		return 1;
	}

	return 0;
}

int rm() {
	printf("Enter Key: ");
	char keyToRemove[10];
	if(!getString(keyToRemove, 10))
		return 0;

	if(rmValue(keyToRemove, kvList, NULL))
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
