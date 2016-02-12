#include <stdio.h>
#include <string.h>

char string[80];

int main(int argc, char* argv[]){
	printf("Enter a string: ");
	scanf("%s", &string);
	
	char command[10];
	do {
		printf("Command: ");
		scanf("%s", &command);
	} while(strcmp(command, "print") != 0);
	
	printf("%s\n", string);
	
	return 0;
}
