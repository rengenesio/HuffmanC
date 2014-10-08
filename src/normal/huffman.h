#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//#define DEBUG 1

#define STDOUT			stdout
#define BYTE 			uint8_t
#define BYTE_BIT		8
#define SIZE			uint8_t
#define SYMBOL			uint8_t

#define POWER_BITS_CODIFICATION			256

#define GET_BYTE(a, b)			*((BYTE *)(a)+(b))
#define SET_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) |= (1 << (BYTE_BIT - 1 - (b) % BYTE_BIT)))
#define CLEAR_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) &= ~(1 << (BYTE_BIT - 1 - (b) % BYTE_BIT)))
#define GET_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) >> ((b) % BYTE_BIT)) & 0x1


typedef struct NODE_ {
	struct NODE_ *left, *right;
	void *symbol;
	unsigned int frequency;
	unsigned char visited;
} NODE;

typedef struct ARRAY_ {
	NODE **node;
	unsigned int size;
} NODE_ARRAY;

typedef struct STACK_ {
	NODE **stack;
	unsigned int top;
} STACK;

typedef struct CODIFICATION_ {
	void *symbol;
	unsigned char *code;
	unsigned int size;
} CODIFICATION;


void encoder(char *file_in, char *file_out, char *file_cb);
void decoder(char *file_in, char *file_out, char *file_cb);

void * fileToMemory(FILE *file, unsigned int *size);
NODE_ARRAY * newNodeArray(unsigned int size);
NODE * newNode(void *symbol, unsigned long int frequency, NODE *left, NODE *right);
int removeNode (NODE_ARRAY *a, NODE *n);
STACK * newStack(unsigned int size);
NODE * pop (STACK *s);
int push (STACK *s, NODE *n);
unsigned int strlenU(unsigned char * str);

int insertSorted (NODE_ARRAY *a, NODE *n);

unsigned int * fileToFrequency(FILE *in, unsigned int *symbols, SYMBOL *eof);
NODE_ARRAY * frequencyToNodeAray(unsigned int * frequency, unsigned int symbols);
int huffmanEncode (NODE_ARRAY *array);
NODE * treeFromCode (unsigned char ** code);
CODIFICATION * treeToCode (NODE *root, unsigned int symbols);
void codificationToFile(FILE *file, CODIFICATION *codification, unsigned int symbols, SYMBOL *eof);
void fileCompressor(FILE *in, CODIFICATION *codification, unsigned int symbols, FILE *file, SYMBOL *eof);

CODIFICATION * fileToCode (FILE *cod, unsigned int *symbols);
NODE * codeToTree (CODIFICATION *codification, unsigned int symbols);
void huffmanDecode(FILE *out, FILE *in, NODE *root, SYMBOL *eof);

void openFiles (FILE **a, char *path_a, char *arg_a, FILE **b, char *path_b, char *arg_b, FILE **c, char *path_c, char *arg_c);
unsigned int power (unsigned int a, unsigned int b);
