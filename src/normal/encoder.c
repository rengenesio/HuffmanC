#include "huffman.h"


void encoder(char *file_in, char *file_out, char *file_cb) {
	FILE *in = NULL;
	FILE *out = NULL;
	FILE *cod = NULL;
	unsigned int symbols;
	void *eof = malloc(sizeof(SYMBOL));
	unsigned int *frequency;
	NODE_ARRAY *node_array;
	CODIFICATION *codification;

	openFiles(&in, file_in, "rb", &out, file_out, "wb", &cod, file_cb, "wb");
#ifdef DEBUG
	fprintf(stderr, "\nFILES:\n\nIn: %s\nOut: %s\nCB: %s\n", argv[1], argv[2], argv[3]);
#endif
	frequency = fileToFrequency(in, &symbols, eof);
#ifdef DEBUG
	fprintf(stderr, "\n\nFREQUENCY: symbol (frequency)\n\n");
	for(unsigned int i = 0 ; i < POWER_BITS_CODIFICATION ; i++)
		if(frequency[i])
			fprintf(stderr, "%u (%u)\n", i, frequency[i]);
	fprintf(stderr, "EOF: %u\n", *((unsigned int *)eof));
#endif
	node_array = frequencyToNodeAray(frequency, symbols);
#ifdef DEBUG
	fprintf(stderr, "\n\nNODE ARRAY: symbol (frequency)\n\n");
	for(unsigned int i = 0 ; i < node_array->size ; i++)
		fprintf(stderr, "%u (%u)\n", *((unsigned int *)node_array->node[i]->symbol), node_array->node[i]->frequency);
#endif
	huffmanEncode(node_array);
	codification = treeToCode(node_array->node[0], symbols);
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
	for(unsigned int i = 0 ; i < symbols ; i++)
		fprintf(stderr, "i: %u - %x (%u) %s\n", i, *((unsigned int *)codification[i].symbol), codification[i].size, codification[i].code);
#endif
	codificationToFile(cod, codification, symbols, eof);
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION FILE WRITTEN!\n");
#endif
	fileCompressor(in, codification, symbols, out, eof);
#ifdef DEBUG
	fprintf(stderr, "\n\nOUTPUT FILE WRITTEN!\n");
#endif

	fclose(in);
	fclose(out);
	fclose(cod);
}
