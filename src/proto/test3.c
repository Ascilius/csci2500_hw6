#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_arr(char* strs[], int size) {
	for (int i = 0; i < size; ++i)
		printf("%s\n", strs[i]);
}

void modify_arr(char* strs[], int size) {
	for (int i = 0; i < size; ++i)
		strs[i] = "Hello World!";
}

int main() {
	int size = 2;
	char* strs[] = {"Hello", "World"};

	print_arr(strs, size);

	modify_arr(strs, size);

	print_arr(strs, size);

	return 0;
}