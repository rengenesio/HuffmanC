#include "huffman.h"

FILE *in_dec = NULL;
FILE *out_dec = NULL;
FILE *cod_dec = NULL;

SYMBOL eof_dec;
SIZE symbols_dec = 0;
NODE *root_dec;
CODIFICATION *codification_dec;
CODIFICATION_ARRAY_ELEMENT *treeArray_dec;
unsigned int max_code_dec;

pthread_mutex_t mutex_dec;
pthread_barrier_t barrier_dec;


void * threadTaskDec(void *tid);
void initializeVarsDec (void);
CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (unsigned int tid);

void decoder(char *file_in, char *file_out, char *file_cb) {
	pthread_t thread[NTHREADS];
	unsigned int tid[NTHREADS];

	initializeVarsDec();
	openFiles(&in_dec, file_in, "rb", &out_dec, file_out, "wb", &cod_dec, file_cb, "rb");
	codification_dec = fileToCode(cod_dec, &symbols_dec, &max_code_dec);
	eof_dec = codification_dec[symbols_dec-1].symbol;
	treeArray_dec = malloc(sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code_dec + 1)));
	memset(treeArray_dec, 0, sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code_dec + 1)));
	newNode(root_dec, 0, 0, NULL, NULL);

#ifdef DEBUG
		fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
		for(unsigned int i = 0 ; i < symbols_dec ; i++)
			fprintf(stderr, "%x (%u) %s\n", codification_dec[i].symbol, codification_dec[i].size, codification_dec[i].code);
		fprintf(stderr, "EOF: %x\n", eof_dec);
#endif

	for(unsigned int i = 0 ; i < NTHREADS ; i++) {
		tid[i] = i;
		if(pthread_create(&thread[i], NULL, threadTaskDec, (void *) &tid[i])) {
			fprintf(STDOUT, "Erro!\n");
			exit(0);
		}
	}

	for(unsigned int i = 0 ; i < NTHREADS ; i++)
		pthread_join(thread[i], NULL);

	huffmanDecode(in_dec, out_dec, treeArray_dec, eof_dec);

	fclose(in_dec);
	fclose(out_dec);
	fclose(cod_dec);

	pthread_exit(NULL);
}

void * threadTaskDec (void *tid) {
	unsigned int my_tid = *((unsigned int *) tid);
	codeToTreeArray(my_tid);
	pthread_exit(NULL);
}

void initializeVarsDec (void) {
	root_dec = malloc(sizeof(NODE));

	if(pthread_mutex_init(&mutex_dec, NULL)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'mutex'!\n");
		exit(0);
	}
	if(pthread_barrier_init (&barrier_dec, NULL, NTHREADS)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'barrier'!\n");
		exit(0);
	}
}

CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (unsigned int tid) {
	unsigned int start, blocks;
	divideArray(symbols_dec, tid, &start, &blocks);

	for(unsigned int i = 0 ; i < blocks ; i++) {
		unsigned int index = 0;
		for(unsigned int j = 0 ; j < codification_dec[start + i].size ; j++) {
			index <<= 1;
			if(codification_dec[start + i].code[j] == '0')
				index += 1;
			else
				index += 2;
		}
		treeArray_dec[index].symbol = codification_dec[start + i].symbol;
		treeArray_dec[index].used = 1;
	}

	return treeArray_dec;
}


