#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Parametros invalidos\n");
		exit(0);
	}

	char nthreads[10]; nthreads[0] = '\0';
	sprintf(nthreads, "%d", NTHREADS);

	char in[50]; in[0] = '\0';
	strcat(in, "../../../Files/");
	strcat(in, argv[1]);

	char out[50]; out[0] = '\0';
	strcat(out, in); strcat(out, ".cdir/thread/"); strcat(out, nthreads); strcat(out, "/compressed");

	char cb[50]; cb[0] = '\0';
	strcat(cb, in); strcat(cb, ".cdir/thread/"); strcat(cb, nthreads); strcat(cb, "/codification");

	encoder(in, out, cb);

	strcat(in, ".cdir/thread/decompressed");
	decoder(out, in, cb);

	return 0;
}
