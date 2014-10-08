#include "huffman.h"

FILE *in = NULL;
FILE *out = NULL;
FILE *cod = NULL;

SYMBOL eof;
SIZE symbols = 0;
NODE *root;
CODIFICATION *codification;
CODIFICATION_ARRAY_ELEMENT *treeArray;
unsigned int max_code;

pthread_mutex_t mutex;
pthread_barrier_t barrier;


void * threadTask(void *tid);
void initializeVars (void);
CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (unsigned int tid);

int main(int argc, char *argv[]) {
	pthread_t thread[NTHREADS];
	unsigned int tid[NTHREADS];

	initializeVars();
	openFiles(&in, argv[1], "rb", &out, argv[2], "wb", &cod, argv[3], "rb");
	codification = fileToCode(cod, &symbols, &max_code);
	eof = codification[symbols-1].symbol;
	treeArray = (CODIFICATION_ARRAY_ELEMENT *) malloc(sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code + 1)));
	memset(treeArray, 0, sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code + 1)));
	newNode(root, 0, 0, NULL, NULL);

#ifdef DEBUG
		fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
		unsigned int i;
		for(i = 0 ; i < symbols ; i++)
			fprintf(stderr, "%x (%u) %s\n", codification[i].symbol, codification[i].size, codification[i].code);
		fprintf(stderr, "EOF: %x\n", eof);
#endif

	unsigned int i;
	for(i = 0 ; i < NTHREADS ; i++) {
		tid[i] = i;
		if(pthread_create(&thread[i], NULL, threadTask, (void *) &tid[i])) {
			fprintf(STDOUT, "Erro!\n");
			exit(0);
		}
	}

	for(i = 0 ; i < NTHREADS ; i++)
		pthread_join(thread[i], NULL);

	huffmanDecode(in, out, treeArray, eof);

	fclose(in);
	fclose(out);
	fclose(cod);

	pthread_exit(NULL);

	return 0;
}

void * threadTask (void *tid) {
	unsigned int my_tid = *((unsigned int *) tid);
	codeToTreeArray(my_tid);
	pthread_exit(NULL);
}

void initializeVars (void) {
	root = (NODE *) malloc(sizeof(NODE));

	if(pthread_mutex_init(&mutex, NULL)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'mutex'!\n");
		exit(0);
	}
	if(pthread_barrier_init (&barrier, NULL, NTHREADS)) {
		fprintf(STDOUT, "Erro ao inicializar a variavel 'barrier'!\n");
		exit(0);
	}
}

CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (unsigned int tid) {
	unsigned int start, blocks;
	divideArray(symbols, tid, &start, &blocks);

	unsigned int i;
	for(i = 0 ; i < blocks ; i++) {
		unsigned int index = 0;
		unsigned int j;
		for(j = 0 ; j < codification[start + i].size ; j++) {
			index <<= 1;
			if(codification[start + i].code[j] == '0')
				index += 1;
			else
				index += 2;
		}
		treeArray[index].symbol = codification[start + i].symbol;
		treeArray[index].used = 1;
	}

	return treeArray;
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
	*blocks = size / NTHREADS;
	*blocks = tid < (size % NTHREADS) ? *blocks+1 : *blocks;
	//*blocks /= coefficient;

	//size /= coefficient;
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
