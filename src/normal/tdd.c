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
	printf("\nSize of NODE: %lu\n", sizeof(NODE));
	printf("Size of ARRAY: %lu\n", sizeof(NODE_ARRAY));
	printf("Size of STACK: %lu\n", sizeof(STACK));
	printf("Size of CODIFICATION: %lu\n", power(2, 2));

	/*unsigned int i = 0;
	long unsigned int li = 0;
	long long unsigned int lli = 0;

	printf("%u   %lu   %llu\n", i, li, lli);

	i--; li--; lli--;
	printf("%u   %lu   %llu\n", i, li, lli);*/

	/*unsigned int a = 1;
	a |= 0xFFFF;
	a -= 2;
	unsigned short int b = 0;

	memcpy(&b, &a, 2);

	printf("%d\n", b);*/

	return 0;
}

void array (void) {
	return;
}

void tree (void) {
	return;
}
