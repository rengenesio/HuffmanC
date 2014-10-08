#include "huffman.h"


void decoder(char *file_in, char *file_out, char *file_cb) {
	FILE *in = NULL;
	FILE *out = NULL;
	FILE *cod = NULL;

	SYMBOL eof;
	SIZE symbols;
	CODIFICATION *codification;
	CODIFICATION_ARRAY_ELEMENT *treeArray;
	unsigned int max_code;

	openFiles(&in, file_in, "rb", &out, file_out, "wb", &cod, file_cb, "rb");
#ifdef DEBUG
	fprintf(stderr, "\nFILES:\n\nIn: %s\nOut: %s\nCB: %s\n", file_in, file_out, file_cb);
#endif
	codification = fileToCode(cod, &symbols, &max_code);
	eof = codification[symbols-1].symbol;
#ifdef DEBUG
	fprintf(stderr, "\n\nCODIFICATION: symbol (size) code\n\n");
	for(unsigned int i = 0 ; i < symbols ; i++)
		fprintf(stderr, "i: %u - %x (%u) %s\n", i, codification[i].symbol, codification[i].size, codification[i].code);
	fprintf(stderr, "EOF: %u\n", eof);
#endif
	treeArray = codeToTreeArray(codification, symbols, max_code);
#ifdef DEBUG
	fprintf(stderr, "\n\nTREE ARRAY: index (symbol)\n");
	for(unsigned int i = 0 ; i < power(2, (max_code + 1)) ; i++)
		if(treeArray[i].used)
			fprintf(stderr, "Index: %u   Symbol: %u\n", i, treeArray[i].symbol);
#endif
	huffmanDecode(in, out, treeArray, eof);

	fclose(in);
	fclose(out);
	fclose(cod);
}
