#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	/*
	// TOFIX: doesn't work :(
	char* str = "Hello World!\n";
	int num = strspn(str, "\n");
	printf("%d\n", num);
	*/

	int len = strspn("geeks for geeks", "geek");
	printf("Length of initial segment matching: %d\n", len);

	len = strcspn("geeks for geeks\n", "\n");
	printf("Length of initial segment matching: %d\n", len);

	// char* buffer = "Hello World!\n";
	char buffer[] = "Hello World!\n";
	buffer[strcspn(buffer, "\n")] = '\0'; 
	printf("%s\n", buffer); // strCspn, not strspn

	return 0;
}