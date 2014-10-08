#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Huffman.h"


int main (void) {
	unsigned int array[100];

	memset(array+4, 0, 96 * sizeof(int));

	for(unsigned int i = 0 ; i < 100 ; i++)
		if(array[i])
			fprintf(stderr, "Array[%u] = %u\n", i, array[i]);

	return 0;
}
