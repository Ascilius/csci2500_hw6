#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_strings(char strings[][16], int size) {
	for (int i = 0; i < size; ++i)
		printf("%p: %s\n", &(strings[i]), strings[i]);
}

void modify_strings(char strings[][16], int size) {
	for (int i = 0; i < size; ++i)
		strcpy(strings[i], "balls");
}

int main() {
	int size = 3;
	char strings[][16] = {"Hello", "Good", "World"};
	printf("%p\n", &strings);

	print_strings(strings, size);

	modify_strings(strings, size);

	print_strings(strings, size);

	return 0;
}