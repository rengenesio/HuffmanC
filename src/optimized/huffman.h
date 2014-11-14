#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//#define DEBUG						1

#define STDOUT						stderr
#define BYTE 						uint8_t
#define FREQUENCY					uint32_t /* Frequência máxima que um símbolo pode ter */

/* CODIFICAÇÃO 8 BITS */

#define SYMBOL						uint8_t
#define SIZE						uint16_t // Imediatamente maior que SYMBOL
#define POWER_SIZE					uint16_t // Imediatamente maior que SIZE
#define FILE_SIZE					uint64_t // Tamanho máximo do arquivo
#define BITS_CODIFICATION			8
#define BYTE_BIT					8
#define SIZEOF_SYMBOL				1
#define POWER_BITS_CODIFICATION		256
#define ALIGN_NODE							2
#define ALIGN_NODE_ARRAY					6
#define ALIGN_TREE_ARRAY					6
#define ALIGN_STACK							6
#define ALIGN_CODIFICATION					5
#define ALIGN_CODIFICATION_ARRAY_ELEMENT	6

/* CODIFICAÇÃO 16 BITS */
/*
#define SYMBOL						uint16_t
#define SIZE						uint32_t // Imediatamente maior que SYMBOL
#define POWER_SIZE					uint32_t // Imediatamente maior que SIZE
#define FILE_SIZE					uint32_t // Tamanho máximo do arquivo
#define BITS_CODIFICATION			16
#define BYTE_BIT					8
#define SIZEOF_SYMBOL				2
#define POWER_BITS_CODIFICATION		65536
#define ALIGN_NODE							1
#define ALIGN_NODE_ARRAY					4
#define ALIGN_TREE_ARRAY					4
#define ALIGN_STACK							4
#define ALIGN_CODIFICATION					2
#define ALIGN_CODIFICATION_ARRAY_ELEMENT	1
*/
#define GET_BYTE(a, b)			*((BYTE *)(a)+(b))
#define SET_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) |= (1 << (BYTE_BIT - 1 - (b) % BYTE_BIT)))
#define CLEAR_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) &= ~(1 << (BYTE_BIT - 1 - (b) % BYTE_BIT)))
#define GET_BIT(a, b)			(GET_BYTE((a), ((b)/BYTE_BIT)) >> (BYTE_BIT - 1 - (b) % BYTE_BIT)) & 0x1


typedef struct NODE_ {
	struct NODE_ *left, *right;
	FREQUENCY frequency;
	SYMBOL symbol;
	uint8_t visited;
	BYTE align[ALIGN_NODE];
} NODE;

typedef struct NODE_ARRAY_ {
	NODE *node;
	SIZE size;
	BYTE align[ALIGN_NODE_ARRAY];
} NODE_ARRAY;

typedef struct TREE_ARRAY_ {
	NODE *node;
	POWER_SIZE size;
	BYTE align[ALIGN_TREE_ARRAY];
} TREE_ARRAY;

typedef struct STACK_ {
	NODE *stack;
	POWER_SIZE top;
	BYTE align[ALIGN_STACK];
} STACK;

typedef struct CODIFICATION_ {
	unsigned char *code;
	SIZE size;
	SYMBOL symbol;
	BYTE align[ALIGN_CODIFICATION];
} CODIFICATION;

typedef struct CODIFICATION_ARRAY_ELEMENT_ {
	SYMBOL symbol;
	uint8_t used;
	BYTE align[ALIGN_CODIFICATION_ARRAY_ELEMENT];
} CODIFICATION_ARRAY_ELEMENT;


void encoder(char *file_in, char *file_out, char *file_cb);
void decoder(char *file_in, char *file_out, char *file_cb);

/* FUNÇÕES ENCODER E DECODER */
void openFiles (FILE **a, char *path_a, char *arg_a, FILE **b, char *path_b, char *arg_b, FILE **c, char *path_c, char *arg_c);
unsigned int strlenU(unsigned char * str);
void newNode (NODE *node, SYMBOL symbol, FREQUENCY frequency, NODE *left, NODE *right);
NODE_ARRAY * newNodeArray(SIZE size);
void insertSorted (NODE_ARRAY *a, NODE *n);
void removeLastNodes (NODE_ARRAY *a, SIZE num);
STACK * newStack(POWER_SIZE size);
NODE * pop (STACK *s);
void push (STACK *s, NODE *n);
unsigned int power (unsigned int a, unsigned int b);
void printTree (NODE *root, SIZE symbols);

/* FUNÇÕES ENCODER */
SYMBOL * fileToMemory(FILE *file, FILE_SIZE *size);
FREQUENCY * memoryToFrequency(SYMBOL *memory, FILE_SIZE *size, SIZE *symbols, SYMBOL *eof);
NODE_ARRAY * frequencyToNodeAray(FREQUENCY * frequency, SIZE symbols);
void huffmanEncode (NODE_ARRAY *array);
CODIFICATION * treeToCode (NODE *root, SIZE symbols);
void codificationToFile(FILE *file, CODIFICATION *codification, SIZE symbols, SYMBOL eof);
void memoryCompressor(SYMBOL *memory, FILE_SIZE size, CODIFICATION *codification, SIZE symbols, SYMBOL eof, FILE *out);

/* FUNÇÕES DECODER */
CODIFICATION * fileToCode (FILE *file, SIZE *symbols, unsigned int *max_code);
CODIFICATION_ARRAY_ELEMENT * codeToTreeArray (CODIFICATION *codification, SIZE symbols, unsigned int max_code);
void huffmanDecode(FILE *in, FILE *out, CODIFICATION_ARRAY_ELEMENT *treeArray, SYMBOL eof);
