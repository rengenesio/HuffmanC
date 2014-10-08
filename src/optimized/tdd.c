#include "Huffman.h"

void array (void);
void tree (void);

int main (int argc, char *argv[]) {
	printf("Size of char: %lu\n", sizeof(char));
	printf("Size of int: %lu\n", sizeof(int));
	printf("Size of short short int: %lu\n", sizeof(short int));
	printf("Size of short int: %lu\n", sizeof(short int));
	printf("Size of long int: %lu\n", sizeof(long int));
	printf("Size of long long int: %lu\n", sizeof(long long int));
	printf("Size of float: %lu\n", sizeof(float));
	printf("Size of double: %lu\n", sizeof(double));
	printf("Size of pointer *: %lu\n", sizeof(int *));
	printf("Size of ARRAY: %lu\n", sizeof(NODE_ARRAY));

	unsigned int a = 0x01020408;
	fprintf(stderr, "%u\n", GET_BIT(&a, 0));
	fprintf(stderr, "%u\n", GET_BIT(&a, 1));
	fprintf(stderr, "%u\n", GET_BIT(&a, 2));
	fprintf(stderr, "%u\n", GET_BIT(&a, 3));
	fprintf(stderr, "%u\n", GET_BIT(&a, 4));
	fprintf(stderr, "%u\n", GET_BIT(&a, 5));
	fprintf(stderr, "%u\n", GET_BIT(&a, 6));
	fprintf(stderr, "%u\n", GET_BIT(&a, 7));

	return 0;
}

void array (void) {
	return;
}

void tree (void) {
	return;
}
