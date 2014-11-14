#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"

int main(int argc, char *argv[]) {
        struct timeval t1, t2;
        double t;
	if(argc != 2) {
		printf("Parametros invalidos\n");
		exit(0);
	}

	char in[100]; in[0] = '\0';
	strcat(in, argv[1]);

	char out[100]; out[0] = '\0';
	strcat(out, in); strcat(out, ".cnordir/compressed");

	char cb[100]; cb[0] = '\0';
	strcat(cb, in); strcat(cb, ".cnordir/codification");

	gettimeofday(&t1, NULL);
        encoder(in, out, cb);
        gettimeofday(&t2, NULL);

        t = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        t += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        t /= 1000;
        printf("%lf s (encoder)\n", t);

	strcat(in, ".cnordir/decompressed");
	gettimeofday(&t1, NULL);
       	decoder(out, in, cb);
        gettimeofday(&t2, NULL);

        t = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        t += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        t /= 1000;
        printf("%lf s (decoder)\n", t);

	return 0;
}

