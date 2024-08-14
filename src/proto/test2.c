#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	if (argc < 3)
		exit(0);

	char str[128]; // original string
	strcpy(str, argv[1]);
	
	char delim[8]; // string of delimiter chars
	strcpy(delim, argv[2]);

	char* token = strtok(str, delim); // get first token

	// traverse through all tokens
	while (token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, delim); // get next token
	}

	return 0;
}