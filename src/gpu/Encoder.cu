#include "huffman.h"


#define BLOCKS_PER_GRID		1
#define THREADS_PER_BLOCK	512
#define TOTAL_THREADS		(BLOCKS_PER_GRID * THREADS_PER_BLOCK)



void * threadTask(void *tid);
void initializeVars(void);
__global__ void frequencyErase (FREQUENCY *f);
__global__ void memoryToFrequency (SYMBOL *m, FREQUENCY *f, FILE_SIZE s);
__global__ void addFrequency (FREQUENCY *f, FREQUENCY *f_total);
void findEof (void);
void frequencyToNodeArray(void);
void treeToCode (void);
__global__ void fileCompressor(unsigned long long int *written_bits, BYTE *memory_out, CODIFICATION *codification, SYMBOL *cuda_memory_in, FILE_SIZE s, SIZE symbols);
void memoryToFile (void);


FILE *in = NULL;
FILE *out = NULL;
FILE *cod = NULL;

SYMBOL eof;
SYMBOL *memory_in, *cuda_memory_in;
FILE_SIZE size_in;
BYTE *memory_out, *cuda_memory_out;
SIZE symbols = 0;
SIZE total_codes = 0;
unsigned long long int *written_bits, *cuda_written_bits;
FREQUENCY *frequency, *frequency_total, *cuda_frequency, *cuda_frequency_total;
NODE_ARRAY *node_array;
CODIFICATION *codification;


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
pthread_barrier_t barrier;


int main(int argc, char *argv[]) {
	unsigned int i;

	openFiles(&in, argv[1], "rb", &out, argv[2], "wb", &cod, argv[3], "wb");

//#ifdef DEBUG
	fprintf(stderr, "\nFILES:\n\nIn: %s\nOut: %s\nCB: %s\n", argv[1], argv[2], argv[3]);
//#endif
	memory_in = fileToMemory(in, &size_in);
//#ifdef DEBUG
	fprintf(stderr, "\n\nSIZE:\n%u bytes\n", size_in);
//#endif

	if(cudaMalloc(&cuda_frequency, sizeof(FREQUENCY) * TOTAL_THREADS * POWER_BITS_CODIFICATION) != cudaSuccess) { printf("Erro cudaMalloc cuda_frequency\n"); return -1; }
	dim3 dim_grid(16, 16, 1);
	dim3 dim_block(32, 16);
	frequencyErase <<< dim_grid , dim_block  >>> (cuda_frequency);

//#ifdef DEBUG
	if((frequency = (FREQUENCY *) malloc(sizeof(FREQUENCY) * TOTAL_THREADS * POWER_BITS_CODIFICATION)) == NULL) { printf("Erro malloc frequency\n"); return -1; }
	if(cudaMemcpy(frequency, cuda_frequency, sizeof(FREQUENCY) * TOTAL_THREADS * POWER_BITS_CODIFICATION, cudaMemcpyDeviceToHost) != cudaSuccess) { printf("Erro cudaMemcpy cuda_frequency\n"); return -1; }

	fprintf(stderr, "\n\nFREQUENCY (IF NOT ERASED): symbol (frequency)\n\n");
	for(i = 0 ; i < TOTAL_THREADS * POWER_BITS_CODIFICATION ; i++)
		if(frequency[i])
			fprintf(stderr, "%u (%u)\n", i, frequency[i]);
//#endif

	if(cudaMalloc(&cuda_memory_in, sizeof(SYMBOL) * size_in) != cudaSuccess) { printf("Erro cudaMalloc cuda_memory_in\n"); return -1; }
	if(cudaMemcpy(cuda_memory_in, memory_in, sizeof(SYMBOL) * size_in, cudaMemcpyHostToDevice) != cudaSuccess) { printf("Erro cudaMemcpy cuda_memory_in\n"); return -1; }
	memoryToFrequency <<< BLOCKS_PER_GRID , THREADS_PER_BLOCK >>> (cuda_memory_in, cuda_frequency, size_in);

	if(cudaMalloc(&cuda_frequency_total, sizeof(FREQUENCY) * POWER_BITS_CODIFICATION) != cudaSuccess) { printf("Erro cudaMalloc cuda_frequency_total\n"); return -1; }
	addFrequency <<< 1 , POWER_BITS_CODIFICATION >>> (cuda_frequency, cuda_frequency_total);

//#ifdef DEBUG
	if((frequency_total = (FREQUENCY *) malloc(sizeof(FREQUENCY) * POWER_BITS_CODIFICATION)) == NULL) { printf("Erro malloc frequency_total\n"); return -1; }
	if(cudaMemcpy(frequency_total, cuda_frequency_total, sizeof(FREQUENCY) * POWER_BITS_CODIFICATION, cudaMemcpyDeviceToHost) != cudaSuccess) { printf("Erro cudaMemcpy cuda_frequency_total\n"); return -1; }
	fprintf(stderr, "\n\nFREQUENCY_TOTAL (IF NOT ERASED): symbol (frequency_total)\n\n");
			
	for(i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
		if(frequency_total[i])
			fprintf(stderr, "%u (%u)\n", i, frequency_total[i]);
//#endif

	findEof();
	node_array = newNodeArray(symbols);
	codification = (CODIFICATION *) malloc(sizeof(CODIFICATION) * symbols);
	if(!codification) {
		fprintf(STDOUT, "Erro ao alocar variavel 'codification'\n");
		exit(-1);
	}

	frequencyToNodeArray();

//#ifdef DEBUG
	fprintf(stderr, "\n\nNODE ARRAY: symbol (frequency)\n\n");
	for(i = 0 ; i < node_array->size ; i++)
		fprintf(stderr, "%u (%u)\n", node_array->node[i].symbol, node_array->node[i].frequency);
//#endif

	huffmanEncode(node_array);
	treeToCode();

//#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
	for(unsigned int i = 0 ; i < symbols ; i++)
		fprintf(stderr, "i: %u - %x (%u) %s\n", i, codification[i].symbol, codification[i].size, codification[i].code);
//#endif

	codificationToFile(cod, codification, symbols, eof);

	if(cudaMalloc(&cuda_written_bits, sizeof(unsigned long long int) * TOTAL_THREADS) != cudaSuccess) { printf("Erro cudaMalloc cuda_written_bits\n"); return -1; }
	if(cudaMalloc(&cuda_memory_out, sizeof(BYTE) * TOTAL_THREADS * size_in) != cudaSuccess) { printf("Erro cudaMalloc cuda_memory_out\n"); return -1; }
	fileCompressor <<< BLOCKS_PER_GRID , THREADS_PER_BLOCK >>> (cuda_written_bits, cuda_memory_out, codification, cuda_memory_in, size_in, symbols);
	memoryToFile();

	return 0;
}

__global__ void frequencyErase (FREQUENCY *f) {
	f[(blockIdx.y * blockDim.y + threadIdx.y) * (blockDim.x * gridDim.x) + (blockIdx.x * blockDim.x + threadIdx.x)] = 0;
}

__global__ void memoryToFrequency (SYMBOL *m, FREQUENCY *f, FILE_SIZE s) {
	unsigned int start, blocks;
	unsigned int tid = threadIdx.x + blockIdx.x * blockDim.x;
	
	blocks = s / TOTAL_THREADS;
	blocks = tid < (s % TOTAL_THREADS) ? blocks+1 : blocks;

	start = (s / TOTAL_THREADS) * tid;
	start = tid >= (s % TOTAL_THREADS) ? start + (s % TOTAL_THREADS) : start + tid;

	unsigned int i;
	for(i = 0 ; i < blocks ; i++)
		f[tid * POWER_BITS_CODIFICATION + m[start + i]]++;
}

__global__ void addFrequency (FREQUENCY *f, FREQUENCY *f_total) {
	unsigned int tid = threadIdx.x;
	f_total[tid] = 0;

	unsigned int i;
	for(i = 0 ; i < TOTAL_THREADS ; i++)
		f_total[tid] += f[(i * POWER_BITS_CODIFICATION) + tid];
}

void findEof (void) {
	symbols = 0;
	unsigned int eof_found = 0;

	unsigned int i;
	for(i = 0 ; i < POWER_BITS_CODIFICATION ; i++) {
		if(frequency_total[i] == 0) {
			if(!eof_found) {
				eof_found = 1;
				eof = i;
				symbols++;
				frequency_total[i]++;
				memset(memory_in + (size_in / SIZEOF_SYMBOL), eof, SIZEOF_SYMBOL);
				size_in += SIZEOF_SYMBOL;
			}
		}
		else
			symbols++;
	}
}

void frequencyToNodeArray (void) {
	unsigned int i;
	for(i = 0 ; i < POWER_BITS_CODIFICATION ; i++) {
		if(frequency_total[i]) {
			NODE *n = (NODE *) malloc(sizeof(NODE));
			newNode(n, i, frequency_total[i], NULL, NULL);
			insertSorted(node_array, n);
		}
	}
}

void treeToCode (void) {
	STACK *s = newStack(2 * POWER_BITS_CODIFICATION - 1);
	NODE *n = &node_array->node[0];
	SIZE codes = 0;

	unsigned char path[(int) POWER_BITS_CODIFICATION];
	memset(path, '\0', POWER_BITS_CODIFICATION);

	push(s, n);
	while(codes < symbols) {
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
			codification[codes].size = strlenU(path);
			codification[codes].symbol = n->symbol;
			codification[codes].code = (unsigned char *) malloc(sizeof(unsigned char) * (codification[codes].size + 1));
			if(!codification[codes].code) {
				fprintf(STDOUT, "Erro ao alocar variavel 'codification[%u].code'\n", codes);
				exit(-1);
			}
			memcpy(codification[codes].code, path, codification[codes].size + 1);
			path[strlenU(path)-1] = '\0';
			n = pop(s);
			codes++;
		}
	}
}

__global__ void fileCompressor(unsigned long long int *wb, BYTE *mo, CODIFICATION *cod, SYMBOL *mi, FILE_SIZE s, SIZE sym) {
	unsigned int start, blocks;
	unsigned int tid = threadIdx.x + blockIdx.x * blockDim.x;
	
	blocks = s / TOTAL_THREADS;
	blocks = tid < (s % TOTAL_THREADS) ? blocks+1 : blocks;

	start = (s / TOTAL_THREADS) * tid;
	start = tid >= (s % TOTAL_THREADS) ? start + (s % TOTAL_THREADS) : start + tid;

	BYTE buffer = 0;
	wb[tid] = 0;

	unsigned int i;
	for(i = 0 ; i < blocks ; i++) {
		unsigned int j;
		for(j = 0 ; j < sym ; j++) {
			if(mi[start + i] == cod[j].symbol) {
				unsigned int tam_string;
				for(tam_string = 0 ; cod[j].code[tam_string] != '\0' ; i++);
				tam_string--;
				unsigned int k;
				for(k = 0 ; k < tam_string ; k++) {
					if(cod[j].code[k] == '1') {
						buffer++;
						wb[tid]++;
						if(wb[tid] == 8)
							buffer = 0;
					}
	
					/*if(cod[j].code[k] == '1')
						SET_BIT(mo[tid * s], wb[tid]);
					else
						CLEAR_BIT(mo[tid * s], wb[tid]);

					wb[tid]++;*/
				}
				break;
			}
		}
	}
}

void memoryToFile (void) {
	BYTE buffer;
	unsigned short int bits = 0;

	unsigned int i;
	for(i = 0 ; i < TOTAL_THREADS ; i++) {
		unsigned int j;
		for(j = 0 ; j < written_bits[i] ; j++) {
			if(GET_BIT(memory_out[i], j))
				SET_BIT(&buffer, bits);
			else
				CLEAR_BIT(&buffer, bits);

			if(++bits == BYTE_BIT) {
				fwrite(&buffer, 1, 1, out);
				bits = 0;
			}
		}
	}
	if(bits)
		fwrite(&buffer, 1, 1, out);
}














/* huffman.c */

/* FUNÇÕES ENCODER E DECODER */
void openFiles (FILE **a, char *path_a, char *arg_a, FILE **b, char *path_b, char *arg_b, FILE **c, char *path_c, char *arg_c) {
	*a = fopen(path_a, arg_a);
	if(*a == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_a);
		exit(0);
	}
	*b = fopen(path_b, arg_b);
	if(*b == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_b);
		exit(0);
	}
	*c = fopen(path_c, arg_c);
	if(*c == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_c);
		exit(0);
	}
}

unsigned int strlenU(unsigned char * str) {
	unsigned int i;
	for(i = 0 ; (*(str++)) ; i++);
	return i;
}

void newNode (NODE *node, SYMBOL symbol, FREQUENCY frequency, NODE *left, NODE *right) {
	node->symbol = symbol;
	node->frequency = frequency;
	if(left) {
		node->left = (NODE *) malloc(sizeof(NODE));
		*(node->left) = *left;
	}
	else
		node->left = NULL;
	if(right) {
		node->right = (NODE *) malloc(sizeof(NODE));
		*(node->right) = *right;
	}
	else
		node->right = NULL;

	node->visited = 0;
}

NODE_ARRAY * newNodeArray (SIZE size) {
	NODE_ARRAY *a = (NODE_ARRAY *) malloc(sizeof(NODE_ARRAY));
	a->node = (NODE *) malloc(sizeof(NODE) * size);
	a->size = 0;

	return a;
}

void insertSorted (NODE_ARRAY *a, NODE *n) {
	SIZE i, j;
	for(i = 0 ; i < a->size && a->node[i].frequency > n->frequency ; i++);
	a->size++;
	for(j = a->size ; j > i ; j--)
		a->node[j] = a->node[j-1];

	a->node[j] = *n;
}

void removeLastNodes (NODE_ARRAY *a, SIZE num) {
	a->size -= num;
}

STACK * newStack (POWER_SIZE size) {
	STACK *s = (STACK *) malloc(sizeof(STACK));
	s->stack = (NODE *) malloc(sizeof(NODE) * size);
	s->top = 0;

	return s;
}

NODE * pop (STACK *s) {
	NODE *n = &(s->stack[s->top-1]);
	s->top--;

	return n;
}

void push (STACK *s, NODE *n) {
	s->stack[s->top] = *n;
	s->top++;
}

unsigned int power (unsigned int a, unsigned int b) {
	unsigned int resultado = 1;

	unsigned int i;
	for (i = 0 ; i < b ; i++)
		resultado *= a;

	return resultado;
}

void divideArray (unsigned int size, unsigned int tid, unsigned int *start, unsigned int *blocks) {
	*blocks = size / BLOCKS_PER_GRID * THREADS_PER_BLOCK;
	*blocks = tid < (size % NTHREADS) ? *blocks+1 : *blocks;

	*start = (size / NTHREADS) * tid;
	*start = tid >= (size % NTHREADS) ? *start + (size % NTHREADS) : *start + tid;
}


/* FUNÇÕES ENCODER */
SYMBOL * fileToMemory(FILE *file, FILE_SIZE *size) {
	SYMBOL *memory;
	fseek (file , 0 , SEEK_END);
	*size = ftell (file);
	rewind (file);

	memory = (SYMBOL *) malloc(*size + (SIZEOF_SYMBOL) + 10);
	if(!memory) {
		fprintf(STDOUT, "Erro ao alocar espaço na memória para o arquivo\n");
		return NULL;
	}

	fread(memory, *size, 1, file);
	return memory;
}

void huffmanEncode (NODE_ARRAY *array) {
	while(array->size > 1) {
		NODE *a, *b, *c;
		a = &(array->node[array->size-2]);
		b = &(array->node[array->size-1]);
		c = (NODE *) malloc(sizeof(NODE));
		newNode(c, 0, a->frequency + b->frequency, a, b);
		//fprintf(stderr, "%u(%u) e %u(%u)  --->   %u(%u)\n", a->symbol, a->frequency, b->symbol, b->frequency, c->symbol, c->frequency);
		removeLastNodes(array, 2);
		insertSorted(array, c);
	}
}

void codificationToFile(FILE *file, CODIFICATION *codification, SIZE symbols, SYMBOL eof) {
	SIZE eof_index = 0;

	SIZE i;
	for(i = 0 ; i < symbols ; i++) {
		if(codification[i].symbol != eof) {
			//fprintf(STDOUT, "%u %u %s\n", codification[i].symbol, codification[i].size, codification[i].code);
			fwrite(&codification[i].symbol, SIZEOF_SYMBOL, 1, file);
			fwrite(&codification[i].size, SIZEOF_SYMBOL, 1, file);
			fwrite(codification[i].code, sizeof(unsigned char), codification[i].size, file);
		}
		else
			eof_index = i;
	}
	//fprintf(STDOUT, "%u %u %s\n", codification[eof_index].symbol, codification[eof_index].size, codification[eof_index].code);
	fwrite(&codification[eof_index].symbol, SIZEOF_SYMBOL, 1, file);
	fwrite(&codification[eof_index].size, SIZEOF_SYMBOL, 1, file);
	fwrite(codification[eof_index].code, sizeof(unsigned char), codification[eof_index].size, file);
}

/* FUNÇÕES DECODER */
CODIFICATION * fileToCode (FILE *file, SIZE *symbols, unsigned int *max_code) {
	CODIFICATION *codification;

	unsigned int read = 1;
	*symbols = 0;
	*max_code = 0;
	codification = (CODIFICATION *) malloc(sizeof(CODIFICATION) * POWER_BITS_CODIFICATION);
	do {
		fread(&codification[*symbols].symbol, SIZEOF_SYMBOL, 1, file);
		fread(&codification[*symbols].size, SIZEOF_SYMBOL, 1, file);
		codification[*symbols].code = (unsigned char *) malloc(sizeof(unsigned char) * (codification[*symbols].size+1));
		read = fread(codification[*symbols].code, sizeof(unsigned char), codification[*symbols].size, file);
		if(codification[*symbols].size > *max_code)
			*max_code = codification[*symbols].size;
		(*symbols)++;
	} while(read);
	(*symbols)--;
	return codification;
}

void huffmanDecode(FILE *in, FILE *out, CODIFICATION_ARRAY_ELEMENT *treeArray, SYMBOL eof) {
	uint8_t feof = 0;
	unsigned int index = 0;

	unsigned int i;
	for(i = 0 ; !feof ; i++) {
		BYTE buffer;
		fread(&buffer, 1, 1, in);
		unsigned int j;
		for(j = 0 ; j < BYTE_BIT ; j++) {
			index <<= 1;
			if((GET_BIT(&buffer, j)) == 0)
				index += 1;
			else
				index += 2;

			if(treeArray[index].used) {
				if(treeArray[index].symbol == eof) {
					feof = 1;
					return;
				}
				else
					fwrite(&treeArray[index].symbol, SIZEOF_SYMBOL, 1, out);
				index = 0;
			}
		}
	}
}
