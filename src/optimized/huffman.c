#include "huffman.h"

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
		node->left = malloc(sizeof(NODE));
		*(node->left) = *left;
	}
	else
		node->left = NULL;
	if(right) {
		node->right = malloc(sizeof(NODE));
		*(node->right) = *right;
	}
	else
		node->right = NULL;

	node->visited = 0;
}

NODE_ARRAY * newNodeArray (SIZE size) {
	NODE_ARRAY *a = malloc(sizeof(NODE_ARRAY));
	a->node = malloc(sizeof(NODE) * size);
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
	s->stack = malloc(sizeof(NODE) * size);
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
	for (unsigned int i = 0 ; i < b ; i++)
		resultado *= a;

	return resultado;
}


/* FUNÇÕES ENCODER */

SYMBOL * fileToMemory(FILE *file, FILE_SIZE *size) {
	SYMBOL *memory;
	fseek (file , 0 , SEEK_END);
	*size = ftell (file);
	rewind (file);

	memory = malloc(*size + (SIZEOF_SYMBOL) + 10);
	if(!memory) {
		fprintf(STDOUT, "Erro ao alocar espaço na memória para o arquivo\n");
		return NULL;
	}

	fread(memory, *size, 1, file);
	return memory;
}

FREQUENCY * memoryToFrequency(SYMBOL *memory, FILE_SIZE *size, SIZE *symbols, SYMBOL *eof) {
	FREQUENCY *frequency = (FREQUENCY *) malloc(sizeof(FREQUENCY) * POWER_BITS_CODIFICATION);
	if(!frequency) {
		fprintf(STDOUT, "Erro ao alocar espaço na memória para a variavel 'frequency'\n");
		return NULL;
	}
	memset(frequency, 0, sizeof(FREQUENCY) * POWER_BITS_CODIFICATION);

	*symbols = 0;
	for(FILE_SIZE i = 0 ; i*(SIZEOF_SYMBOL) < *size ; i++) {
		SYMBOL symbol;
		memcpy(&symbol, memory + i, SIZEOF_SYMBOL);
		if(frequency[symbol]++ == 0)
			(*symbols)++;
	}

	for(SIZE i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
		if(!frequency[i]) {
			*eof = i;
			(*symbols)++;
			frequency[i]++;
			memset(memory + (*size / SIZEOF_SYMBOL), *eof, SIZEOF_SYMBOL);
			(*size) += SIZEOF_SYMBOL;
			break;
		}

	return frequency;
}

NODE_ARRAY * frequencyToNodeAray(FREQUENCY * frequency, SIZE symbols) {
	NODE_ARRAY *node_array = newNodeArray(symbols);

	for(SIZE i = 0 ; i < POWER_BITS_CODIFICATION ; i++) {
		if(frequency[i]) {
			NODE *n = malloc(sizeof(NODE));
			newNode(n, i, frequency[i], NULL, NULL);
			insertSorted(node_array, n);
		}
	}

	return node_array;
}

void huffmanEncode (NODE_ARRAY *array) {
	while(array->size > 1) {
		NODE *a, *b, *c;
		a = &(array->node[array->size-2]);
		b = &(array->node[array->size-1]);
		c = malloc(sizeof(NODE));
		newNode(c, 0, a->frequency + b->frequency, a, b);
		fprintf(stderr, "%u(%u) e %u(%u)  --->   %u(%u)\n", a->symbol, a->frequency, b->symbol, b->frequency, c->symbol, c->frequency);
		removeLastNodes(array, 2);
		insertSorted(array, c);
	}
}

CODIFICATION * treeToCode (NODE *root, SIZE symbols) {
	STACK *s = newStack(2 * POWER_BITS_CODIFICATION - 1);
	CODIFICATION *codification = malloc(sizeof(CODIFICATION) * symbols);
	NODE *n = root;
	SIZE codes = 0;

	unsigned char path[(int) POWER_BITS_CODIFICATION];
	memset(path, '\0', POWER_BITS_CODIFICATION);

	push(s, root);
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

	return codification;
}

void codificationToFile(FILE *file, CODIFICATION *codification, SIZE symbols, SYMBOL eof) {
	SIZE eof_index = 0;
	for(SIZE i = 0 ; i < symbols ; i++) {
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

void memoryCompressor(SYMBOL *memory, FILE_SIZE size, CODIFICATION *codification, SIZE symbols, SYMBOL eof, FILE *out) {
	BYTE buffer;
	unsigned int bits = 0;

	for(unsigned int i = 0 ; i < size / SIZEOF_SYMBOL ; i++) {
		for(unsigned int j = 0 ; j < symbols ; j++) {
			if(memory[i] == codification[j].symbol) {
				unsigned int tam_string = strlenU(codification[j].code);
				for(unsigned int k = 0 ; k < tam_string ; k++) {
					if(codification[j].code[k] == '1')
						SET_BIT(&buffer, bits);
					else
						CLEAR_BIT(&buffer, bits);

					if(++bits == BYTE_BIT) {
						fwrite(&buffer, 1, 1, out);
						bits = 0;
					}
				}
				break;
			}
		}
	}
	if(bits)
		fwrite(&buffer, 1, 1, out);
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
		codification[*symbols].code = malloc(sizeof(unsigned char) * (codification[*symbols].size+1));
		read = fread(codification[*symbols].code, sizeof(unsigned char), codification[*symbols].size, file);
		if(codification[*symbols].size > *max_code)
			*max_code = codification[*symbols].size;
		(*symbols)++;
	} while(read);
	(*symbols)--;
	return codification;
}

CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (CODIFICATION *codification, SIZE symbols, unsigned int max_code) {
	CODIFICATION_ARRAY_ELEMENT *treeArray = malloc(sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code + 1)));
	memset(treeArray, 0, sizeof(CODIFICATION_ARRAY_ELEMENT) * power(2, (max_code + 1)));

	for(unsigned int i = 0 ; i < symbols ; i++) {
		unsigned int index = 0;
		for(unsigned int j = 0 ; j < codification[i].size ; j++) {
			index <<= 1;
			if(codification[i].code[j] == '0')
				index += 1;
			else
				index += 2;
		}
		treeArray[index].symbol = codification[i].symbol;
		treeArray[index].used = 1;
	}

	return treeArray;
}

void huffmanDecode(FILE *in, FILE *out, CODIFICATION_ARRAY_ELEMENT *treeArray, SYMBOL eof) {
	uint8_t feof = 0;
	unsigned int index = 0;
	for(unsigned int i = 0 ; !feof ; i++) {
		BYTE buffer;
		fread(&buffer, 1, 1, in);
		for(unsigned int j = 0 ; j < BYTE_BIT ; j++) {
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
