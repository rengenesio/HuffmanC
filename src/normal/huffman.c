#include "huffman.h"

NODE * newNode (void *symbol, unsigned long int frequency, NODE *left, NODE *right) {
	NODE *n;
	n = (NODE *) malloc(sizeof(NODE));

	if(symbol) {
		n->symbol = malloc(sizeof(SYMBOL));
		memcpy(n->symbol, symbol, sizeof(SYMBOL));
	}
	n->frequency = frequency;
	n->left = left;
	n->right = right;
	n->visited = 0;

	return n;
}

NODE_ARRAY * newNodeArray (unsigned int size) {
	NODE_ARRAY *a;

	a = (NODE_ARRAY *) malloc(sizeof(NODE_ARRAY));

	a->size = 0;
	a->node = (NODE **) malloc(sizeof(NODE *) * size);

	return a;
}

STACK * newStack (unsigned int size) {
	STACK *s;

	s = (STACK *) malloc(sizeof(STACK));

	s->top = 0;
	s->stack = (NODE **) malloc(sizeof(NODE *) * size);

	return s;
}

int insertSorted (NODE_ARRAY *a, NODE *n) {
	int i, j;
	for(i = 0 ; i < a->size && a->node[i]->frequency > n->frequency ; i++);
	a->size++;
	for(j = a->size ; j > i ; j--)
		a->node[j] = a->node[j-1];

	a->node[j] = n;

	return 1;
}

int removeNode (NODE_ARRAY *a, NODE *n) {
	for(int i = 0 ; i < a->size ; i++)
		if(a->node[i] == n)
			for(int j = i ; j < a->size ; j++)
				a->node[j] = a->node[j+1];

	a->size--;

	return 1;
}

NODE * pop (STACK *s) {
	NODE *n;

	if(s->top == 0)
		return NULL;

	n = s->stack[s->top-1];
	s->top--;

	return n;
}

int push (STACK *s, NODE *n) {
	s->stack[s->top] = n;
	s->top++;

	return 1;
}

NODE * treeFromCode (unsigned char ** code) {
	NODE *root = newNode(0, 0, NULL, NULL);
	NODE *n;

	for(int i = 0 ; i < power(2, sizeof(SYMBOL) * BYTE_BIT) ; i++) {
		if(strlenU(code[i])) {
			n = root;
			for(int j = 0 ; j < strlenU(code[i]) ; j++) {
				if(code[i][j] == '0') {
					if(n->left)
						n = n->left;
					else {
						NODE *n2 = newNode(&i, 0, NULL, NULL);
						n->left = n2;
						n = n2;
					}
				}
				if(code[i][j] == '1') {
					if(n->right)
						n = n->right;
					else {
						NODE *n2 = newNode(&i, 0, NULL, NULL);
						n->right = n2;
						n = n2;
					}
				}
			}
		}
	}

	return root;
}

unsigned int * fileToFrequency(FILE *in, unsigned int *symbols, SYMBOL *eof) {
	unsigned int *frequency;
	frequency = (unsigned int *) malloc(sizeof(unsigned int) * power(2, sizeof(SYMBOL) * BYTE_BIT));
	for(int i = 0 ; i < power(2, sizeof(SYMBOL) * BYTE_BIT) ; i++)
		frequency[i] = 0;

	*symbols = 0;
	unsigned int bytes;
	do {
		unsigned int symbol_index = 0;
		bytes = fread(&symbol_index, sizeof(SYMBOL), 1, in);
		if(frequency[symbol_index]++ == 0)
			(*symbols)++;
	} while(bytes);

	for(unsigned int i = 0 ; i < power(2, sizeof(SYMBOL) * BYTE_BIT) ; i++)
		if(!frequency[i]) {
			memcpy(eof, &i, sizeof(SYMBOL));
			(*symbols)++;
			frequency[i]++;
			break;
		}

	return frequency;
}

NODE_ARRAY * frequencyToNodeAray(unsigned int * frequency, unsigned int symbols) {
	NODE_ARRAY *node_array = newNodeArray(symbols);

	for(unsigned int i = 0 ; i < power(2, sizeof(SYMBOL) * BYTE_BIT) ; i++) {
		if(frequency[i]) {
			NODE *n = newNode(&i, frequency[i], NULL, NULL);
			insertSorted(node_array, n);
		}
	}

	return node_array;
}

int huffmanEncode (NODE_ARRAY *array) {
	while(array->size > 1) {
		NODE *a, *b, *c;
		a = array->node[array->size-1];
		b = array->node[array->size-2];
		c = newNode(NULL, a->frequency + b->frequency, b, a);
		removeNode(array, a);
		removeNode(array, b);
		insertSorted(array, c);
	}

	return 1;
}

CODIFICATION * treeToCode (NODE *root, unsigned int symbols) {
	STACK *s;
	CODIFICATION *codification;
	NODE *n = root;

	s = newStack(2 * power(2, sizeof(SYMBOL) * BYTE_BIT) - 1);
	codification = (CODIFICATION *) malloc(sizeof(CODIFICATION) * symbols);
	for(unsigned int i = 0 ; i < symbols ; i++) {
		codification[i].code = (unsigned char *) malloc(sizeof(unsigned char) * power(2, sizeof(SYMBOL) * BYTE_BIT));
		codification[i].symbol = malloc(sizeof(SYMBOL));
	}

	unsigned char path[(int) power(2, sizeof(SYMBOL) * BYTE_BIT)];
	for(unsigned int i = 0 ; i < power(2, sizeof(SYMBOL) * BYTE_BIT) ; i++)
		path[i] = '\0';

	push(s, n);
	unsigned int codes = 0;
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
			memcpy(codification[codes].symbol, n->symbol, sizeof(SYMBOL));
			codification[codes].size = strlenU(path);
			memcpy(codification[codes].code, path, strlenU(path) + 1);
			path[strlenU(path)-1] = '\0';
			n = pop(s);
			codes++;
		}
	}

	return codification;
}

void codificationToFile(FILE *file, CODIFICATION *codification, unsigned int symbols, SYMBOL *eof) {
	unsigned int eof_index = 0;

	for(unsigned int i = 0 ; i < symbols ; i++) {
		if(memcmp(codification[i].symbol, eof, sizeof(SYMBOL)) != 0) {
			fwrite(codification[i].symbol, sizeof(SYMBOL), 1, file);
			fwrite(&(codification[i].size), sizeof(SYMBOL), 1, file);
			fwrite(codification[i].code, sizeof(unsigned char), codification[i].size, file);
		}
		else
			eof_index = i;
	}
	fwrite(codification[eof_index].symbol, sizeof(SYMBOL), 1, file);
	fwrite(&(codification[eof_index].size), sizeof(SYMBOL), 1, file);
	fwrite(codification[eof_index].code, sizeof(unsigned char), codification[eof_index].size, file);
}

void fileCompressor(FILE *in, CODIFICATION *codification, unsigned int symbols, FILE *out, SYMBOL *eof) {
	unsigned int bits = 0;
	BYTE buffer = 0;

	rewind(in);
	unsigned int bytes;
	do {
		SYMBOL symbol;
		bytes = fread(&symbol, sizeof(SYMBOL), 1, in);
		if(!bytes)
			memcpy(&symbol, eof, sizeof(SYMBOL));

		for(unsigned int j = 0 ; j < symbols ; j++) {
			if(memcmp(codification[j].symbol, &symbol, sizeof(SYMBOL)) == 0) {
				for(unsigned int k = 0 ; k < strlenU(codification[j].code) ; k++) {
					if(codification[j].code[k] == '1')
						SET_BIT(&buffer, bits);
					else
						CLEAR_BIT(&buffer, bits);

					bits++;
					if(bits == BYTE_BIT) {
						fwrite(&buffer, sizeof(BYTE), 1, out);
						bits = 0;
					}
				}
				break;
			}
		}
	} while(bytes);

	if(bits)
		fwrite(&buffer, 1, 1, out);
}

unsigned int strlenU(unsigned char * str) {
    unsigned int i;

    for(i = 0 ; (*(str++)) ; i++);

    return i;
}


CODIFICATION * fileToCode (FILE *cod, unsigned int *symbols) {
	CODIFICATION *codification;

	*symbols = 0;
	codification = (CODIFICATION *) malloc(sizeof(CODIFICATION) * power(2, sizeof(SYMBOL) * BYTE_BIT));
	while(1) {
		codification[*symbols].symbol = malloc(sizeof(SYMBOL));
		if(fread(codification[*symbols].symbol, sizeof(SYMBOL), 1, cod) != 1)
			break;
		fread(&codification[*symbols].size, sizeof(SYMBOL), 1, cod);

		codification[*symbols].code = (unsigned char *) malloc(sizeof(unsigned char) * (codification[*symbols].size+1));
		fread(codification[*symbols].code, sizeof(unsigned char), codification[*symbols].size, cod);
		codification[*symbols].code[codification[*symbols].size+1] = '\0';
		//fprintf(STDOUT, "Symbol: %u  Code: %s\n", *((unsigned int *) codification[*symbols].symbol), codification[*symbols].code);

		(*symbols)++;
	}

	return codification;
}

NODE * codeToTree (CODIFICATION *codification, unsigned int symbols) {
	NODE *root = newNode(0, 0, NULL, NULL);
	NODE *n;

	for(unsigned int i = 0 ; i < symbols ; i++) {
		n = root;
		for(unsigned int j = 0 ; j < strlenU(codification[i].code) ; j++) {
			if(codification[i].code[j] == '0') {
				if(n->left)
					n = n->left;
				else {
					NODE *n2 = newNode(codification[i].symbol, 0, NULL, NULL);
					n->left = n2;
					n = n2;
				}
			}
			if(codification[i].code[j] == '1') {
				if(n->right)
					n = n->right;
				else {
					NODE *n2 = newNode(codification[i].symbol, 0, NULL, NULL);
					n->right = n2;
					n = n2;
				}
			}
		}
	}

	return root;
}

void huffmanDecode(FILE *out, FILE *in, NODE *root, SYMBOL *eof) {
	NODE *n = root;
	unsigned int feof = 0;
	for(unsigned int i = 0 ; !feof ; i++) {
		unsigned char buffer;
		fread(&buffer, 1, 1, in);
		//fprintf(STDOUT, "i = %u\n", i);
		for(unsigned int j = 0 ; j < 8 ; j++) {
			unsigned char bit = buffer >> (7-j);
			bit &= 1;
			if(!bit) {
				//fprintf(STDOUT, "bit 0\n");
				if(n->left->left) {
					n = n->left;
				}
				else {
					//fprintf(STDOUT, "Comparando: %u  %u  -  ", *((unsigned int *) eof), *((unsigned int *) n->left->symbol));
					if(memcmp(n->left->symbol, eof, sizeof(SYMBOL)) == 0) {
						//fprintf(STDOUT, "Iguais!\n");
						feof = 1;
						break;
					}
					else {
						//fprintf(STDOUT, "Diferentes!\n");
						fwrite(n->left->symbol, sizeof(SYMBOL), 1, out);
					}

					n = root;
				}
			}
			else {
				//fprintf(STDOUT, "bit 1\n");
				if(n->right->left) {
					n = n->right;
				}
				else {
					//fprintf(STDOUT, "Comparando: %u  %u  -  ", *((unsigned int *) eof), *((unsigned int *) n->right->symbol));
					if(memcmp(n->right->symbol, eof, sizeof(SYMBOL)) == 0) {
						//fprintf(STDOUT, "Iguais!\n");
						feof = 1;
						break;
					}
					else {
						//fprintf(STDOUT, "Diferentes!\n");
						fwrite(n->right->symbol, sizeof(SYMBOL), 1, out);
					}
					n = root;
				}
			}
		}
	}
}

void openFiles (FILE **a, char *path_a, char *arg_a, FILE **b, char *path_b, char *arg_b, FILE **c, char *path_c, char *arg_c) {
	if((*a = fopen(path_a, arg_a)) == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_a);
		exit(0);
	}

	if((*b = fopen(path_b, arg_b)) == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_b);
		exit(0);
	}

	if((*c = fopen(path_c, arg_c)) == NULL) {
		fprintf(STDOUT, "Erro ao abrir o arquivo %s\n", path_c);
		exit(0);
	}
}

unsigned int power (unsigned int a, unsigned int b) {
	unsigned int resultado = 1;
	for (unsigned int i = 0 ; i < b ; i++)
		resultado *= a;

	return resultado;
}
