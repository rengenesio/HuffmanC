#include "huffman.h"


void encoder(char *file_in, char *file_out, char *file_cb) {
	FILE *in = NULL;
	FILE *out = NULL;
	FILE *cod = NULL;

	SYMBOL *memory;
	FILE_SIZE size;
	SYMBOL eof;
	SIZE symbols;
	FREQUENCY *frequency = NULL;
	NODE_ARRAY *node_array;
	CODIFICATION *codification;

	openFiles(&in, file_in, "rb", &out, file_out, "wb", &cod, file_cb, "wb");
#ifdef DEBUG
	fprintf(stderr, "\nFILES:\n\nIn: %s\nOut: %s\nCB: %s\n", file_in, file_out, file_cb);
#endif
	memory = fileToMemory(in, &size);
#ifdef DEBUG
	fprintf(stderr, "\n\nSIZE:\n%u bytes\n", size);
#endif
	frequency = memoryToFrequency(memory, &size, &symbols, &eof);
#ifdef DEBUG
	fprintf(stderr, "\n\nFREQUENCY: symbol (frequency)\n\n");
	for(unsigned int i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
		if(frequency[i])
			fprintf(stderr, "%u (%u)\n", i, frequency[i]);
	fprintf(stderr, "EOF: %u\n", eof);
#endif
	node_array = frequencyToNodeAray(frequency, symbols);
#ifdef DEBUG
	fprintf(stderr, "\n\nNODE ARRAY: symbol (frequency)\n\n");
	for(unsigned int i = 0 ; i < node_array->size ; i++)
		fprintf(stderr, "%u (%u)\n", node_array->node[i].symbol, node_array->node[i].frequency);
#endif
	huffmanEncode(node_array);
	codification = treeToCode(&node_array->node[0], symbols);
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
	for(unsigned int i = 0 ; i < symbols ; i++)
		fprintf(stderr, "i: %u - %x (%u) %s\n", i, codification[i].symbol, codification[i].size, codification[i].code);
#endif
	codificationToFile(cod, codification, symbols, eof);
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION FILE WRITTEN!\n");
#endif
	memoryCompressor(memory, size, codification, symbols, eof, out);
#ifdef DEBUG
	fprintf(stderr, "\n\nOUTPUT FILE WRITTEN!\n");
#endif
	fclose(in);
	fclose(out);
	fclose(cod);
}
