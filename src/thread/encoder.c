#include "huffman.h"


void * threadTask(void *tid);
void initializeVars(void);
void frequencyErase (unsigned int tid);
void memoryToFrequency(unsigned int tid);
void findEof (void);
void frequencyToNodeAray(unsigned int tid);
void treeToCode (unsigned int tid);
void fileCompressor(unsigned int tid);
void memoryToFile (void);


FILE *in_enc = NULL;
FILE *out_enc = NULL;
FILE *cod_enc = NULL;

SYMBOL eof_enc;
SYMBOL *memory_in_enc;
FILE_SIZE size_in_enc;
BYTE *memory_out_enc[NTHREADS];
//BYTE *total_memory;
SIZE symbols_enc = 0;
SIZE total_codes_enc = 0;
unsigned long long int written_bits_enc[NTHREADS];
//unsigned long long int total_bits = 0;
//unsigned long long int total_bytes = 0;
FREQUENCY *frequency_enc;
NODE_ARRAY *node_array_enc;
CODIFICATION *codification_enc;


unsigned char visited1 = 0;
unsigned char visited2 = 0;
unsigned char visited3 = 0;
unsigned char visited4 = 0;

unsigned char visited5 = 0;
unsigned char visited6 = 0;
unsigned char visited7 = 0;
unsigned char visited8 = 0;
unsigned char visited9 = 0;

pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_barrier_t barrier_enc;


void encoder(char *file_in, char *file_out, char *file_cb) {
	pthread_t thread[NTHREADS];
	unsigned int tid[NTHREADS];

	initializeVars();
	openFiles(&in_enc, file_in, "rb", &out_enc, file_out, "wb", &cod_enc, file_cb, "wb");
#ifdef DEBUG
	fprintf(stderr, "\nFILES:\n\nIn: %s\nOut: %s\nCB: %s\n", file_in, file_out, file_cb);
#endif
	memory_in_enc = fileToMemory(in_enc, &size_in_enc);
#ifdef DEBUG
	fprintf(stderr, "\n\nSIZE:\n%u bytes\n", size_in_enc);
#endif
	for(unsigned int i = 0 ; i < NTHREADS ; i++) {
		tid[i] = i;
		if(pthread_create(&thread[i], NULL, threadTask, (void *) &tid[i])) {
			fprintf(STDOUT, "Erro!\n");
			exit(-1);
		}
	}
	pthread_exit(NULL);
}

void * threadTask (void *tid) {
	unsigned int my_tid = *((unsigned int *) tid);

	frequencyErase(my_tid);
	pthread_barrier_wait(&barrier_enc);
#ifdef DEBUG
	pthread_mutex_lock(&mutex);
	if(!visited8) {
		visited8 = 1;
		fprintf(STDOUT, "\n\nFREQUENCY ERASED:\n");
		for(unsigned int i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
			if(frequency_enc[i])
				fprintf(STDOUT, "Frequency not erased: %u (%u)\n", i, frequency_enc[i]);
	}
	pthread_mutex_unlock(&mutex);
#endif
	memoryToFrequency(my_tid);
	pthread_barrier_wait(&barrier_enc);

	pthread_mutex_lock(&mutex);
	if(!visited1) {
		visited1 = 1;
		findEof();
		node_array_enc = newNodeArray(symbols_enc);
		codification_enc = (CODIFICATION *) malloc(sizeof(CODIFICATION) * symbols_enc);
		if(!codification_enc) {
			fprintf(STDOUT, "Erro ao alocar variavel 'codification'\n");
			exit(-1);
		}
#ifdef DEBUG
		fprintf(stderr, "\n\nFREQUENCY: symbol (frequency)\n\n");
		for(unsigned int i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
			if(frequency_enc[i])
				fprintf(stderr, "%u (%u)\n", i, frequency_enc[i]);
		fprintf(stderr, "EOF: %u\n", eof_enc);
#endif
	}
	pthread_mutex_unlock(&mutex);

	frequencyToNodeAray(my_tid);

	pthread_barrier_wait(&barrier_enc);

	pthread_mutex_lock(&mutex);
	if(!visited2) {
		visited2 = 1;
#ifdef DEBUG
	fprintf(stderr, "\n\nNODE ARRAY: symbol (frequency)\n\n");
	for(unsigned int i = 0 ; i < node_array_enc->size ; i++)
		fprintf(stderr, "%u (%u)\n", node_array_enc->node[i].symbol, node_array_enc->node[i].frequency_enc);
#endif
		huffmanEncode(node_array_enc);
	}
	pthread_mutex_unlock(&mutex);

	treeToCode(my_tid);
	pthread_barrier_wait(&barrier_enc);

#ifdef DEBUG
	pthread_mutex_lock(&mutex);
	if(!visited5) {
		visited5 = 1;
		fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
		for(unsigned int i = 0 ; i < symbols_enc ; i++)
			fprintf(stderr, "i: %u - %x (%u) %s\n", i, codification_enc[i].symbol, codification_enc[i].size, codification_enc[i].code);
	}
	pthread_mutex_unlock(&mutex);
#endif
	pthread_mutex_lock(&mutex);
	if(!visited3) {
		visited3 = 1;
		codificationToFile(cod_enc, codification_enc, symbols_enc, eof_enc);
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION FILE WRITTEN!\n");
#endif
	}
	pthread_mutex_unlock(&mutex);

	fileCompressor(my_tid);
#ifdef DEBUG
	fprintf(stderr, "Thread %u: %llu bits\n", my_tid, written_bits_enc[my_tid]);
#endif
	pthread_barrier_wait(&barrier_enc);

	pthread_mutex_lock(&mutex);
	if(!visited4) {
		visited4 = 1;
		memoryToFile();
#ifdef DEBUG
		fprintf(stderr, "\n\nOUTPUT FILE WRITTEN!\n");
#endif
	}
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
	return NULL;
}

void initializeVars (void) {
	if(pthread_mutex_init(&mutex, NULL)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'mutex'!\n");
		exit(0);
	}
	if(pthread_mutex_init(&mutex2, NULL)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'mutex2'!\n");
		exit(0);
	}
	if(pthread_barrier_init (&barrier_enc, NULL, NTHREADS)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'barrier'!\n");
		exit(0);
	}

	frequency_enc = (unsigned int *) malloc(sizeof(unsigned int) * POWER_BITS_CODIFICATION);
	if(!frequency_enc) {
		fprintf(STDOUT, "Erro ao alocar variavel 'frequency'\n");
		exit(-1);
	}
}

void frequencyErase (unsigned int tid) {
	unsigned int start, blocks;
	divideArray(POWER_BITS_CODIFICATION, tid, &start, &blocks);
	memset(frequency_enc + start, 0, blocks * sizeof(FREQUENCY));
}

void memoryToFrequency (unsigned int tid) {
	unsigned int start, blocks;
	divideArray(size_in_enc/SIZEOF_SYMBOL, tid, &start, &blocks);

	for(unsigned int i = 0 ; i < blocks ; i++) {
		pthread_mutex_lock(&mutex);
		if(frequency_enc[memory_in_enc[start + i]]++ == 0)
			symbols_enc++;
		pthread_mutex_unlock(&mutex);
	}
}

void findEof (void) {
	for(unsigned int i = 0 ; i < POWER_BITS_CODIFICATION ; i++) {
		if(frequency_enc[i] == 0) {
			eof_enc = i;
			symbols_enc++;
			frequency_enc[i]++;
			memset(memory_in_enc + (size_in_enc / SIZEOF_SYMBOL), eof_enc, SIZEOF_SYMBOL);
			size_in_enc += SIZEOF_SYMBOL;
			return;
		}
	}
}

void frequencyToNodeAray (unsigned int tid) {
	unsigned int start, blocks;
	divideArray(POWER_BITS_CODIFICATION, tid, &start, &blocks);

	for(unsigned int i = 0 ; i < blocks ; i++) {
		SYMBOL symbol = start + i;
		if(frequency_enc[symbol]) {
			NODE *n = malloc(sizeof(NODE));
			newNode(n, symbol, frequency_enc[symbol], NULL, NULL);
			pthread_mutex_lock(&mutex);
			insertSorted(node_array_enc, n);
			pthread_mutex_unlock(&mutex);
		}
	}
}

void treeToCode (unsigned int tid) {
	STACK *s = newStack(2 * POWER_BITS_CODIFICATION - 1);
	NODE *n = &node_array_enc->node[0];
	unsigned char path[(int) POWER_BITS_CODIFICATION];
	memset(path, '\0', POWER_BITS_CODIFICATION);

	unsigned int cont = 1;
	for(unsigned int i = 0 ; i < LOG_NTHREADS ; i++) {
		if(!((tid >> i) & 0x1)) {
			if(n->left) {
				n = n->left;
				path[i] = '0';
			}
			else {
				pthread_mutex_lock(&mutex2);
				if(!n->visited) {
					n->visited = 1;
					i = LOG_NTHREADS;
				}
				else
					cont = 0;
				pthread_mutex_unlock(&mutex2);
			}
		}
		else {
			if(n->right) {
				n = n->right;
				path[i] = '1';
			}
			else {
				pthread_mutex_lock(&mutex2);
				if(!n->visited) {
					n->visited = 1;
					i = LOG_NTHREADS;
				}
				else
					cont = 0;
				pthread_mutex_unlock(&mutex2);
			}
		}
	}

	if(cont)
		push(s, n);
	else
		return;

	while(s->top != 0) {
		if(n->left) {
			if(!n->left->visited) {
				push(s, n);
				n->visited = 1;
				n = n->left;
				path[strlenU(path)] = '0';
			}
			else if(!n->right->visited) {
				push(s, n);
				n->visited = 1;
				n = n->right;
				path[strlenU(path)] = '1';
			}
			else {
				path[strlenU(path)-1] = '\0';
				n = pop(s);
			}
		}
		else {
			n->visited = 1;
			pthread_mutex_lock(&mutex2);
			codification_enc[total_codes_enc].size = strlenU(path);
			codification_enc[total_codes_enc].symbol = n->symbol;
			codification_enc[total_codes_enc].code = (unsigned char *) malloc(sizeof(unsigned char) * (codification_enc[total_codes_enc].size + 1));
			if(!codification_enc[total_codes_enc].code) {
				fprintf(STDOUT, "Erro ao alocar variavel 'codification[%u].code'\n", total_codes_enc);
				exit(-1);
			}
			memcpy(codification_enc[total_codes_enc].code, path, codification_enc[total_codes_enc].size + 1);
			total_codes_enc++;
			pthread_mutex_unlock(&mutex2);

			path[strlenU(path)-1] = '\0';
			n = pop(s);
		}
	}
}

void fileCompressor(unsigned int tid) {
	unsigned int start, blocks;
	divideArray(size_in_enc/SIZEOF_SYMBOL, tid, &start, &blocks);

	written_bits_enc[tid] = 0;
	memory_out_enc[tid] = (BYTE *) malloc(2 * blocks * sizeof(BYTE));
	if(!memory_out_enc[tid]) {
		fprintf(STDOUT, "Erro ao alocar variavel 'memory_out[%u'\n", tid);
		exit(-1);
	}

	for(unsigned int i = 0 ; i < blocks ; i++) {
		for(unsigned int j = 0 ; j < symbols_enc ; j++) {
			if(memory_in_enc[start + i] == codification_enc[j].symbol) {
				unsigned int tam_string = strlenU(codification_enc[j].code);
				for(unsigned int k = 0 ; k < tam_string ; k++) {
					if(codification_enc[j].code[k] == '1')
						SET_BIT(memory_out_enc[tid], written_bits_enc[tid]);
					else
						CLEAR_BIT(memory_out_enc[tid], written_bits_enc[tid]);

					written_bits_enc[tid]++;
				}
				break;
			}
		}
	}
}

void memoryToFile (void) {
	/*for(unsigned int i = 0 ; i < NTHREADS ; i++)
		total_bits += written_bits[i];
	total_bytes = (total_bits % 8) ? (total_bits/8) + 1 : (total_bits/8);*/

	/*total_memory = malloc(sizeof(BYTE) * total_bytes);
	if(!total_memory) {
		fprintf(STDOUT, "Erro ao alocar variavel 'total_memory'\n");
		exit(-1);
	}*/
	BYTE buffer;
	unsigned short int bits = 0;
	for(unsigned int i = 0 ; i < NTHREADS ; i++) {
		for(unsigned int j = 0 ; j < written_bits_enc[i] ; j++) {
			if(GET_BIT(memory_out_enc[i], j))
				SET_BIT(&buffer, bits);
			else
				CLEAR_BIT(&buffer, bits);

			if(++bits == BYTE_BIT) {
				fwrite(&buffer, 1, 1, out_enc);
				bits = 0;
			}
		}
	}
	if(bits)
		fwrite(&buffer, 1, 1, out_enc);
}
