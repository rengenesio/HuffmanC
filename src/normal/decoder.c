#include "huffman.h"

void decoder(char *file_in, char *file_out, char *file_cb) {
	FILE *in = NULL;
	FILE *out = NULL;
	FILE *cod = NULL;
	unsigned int symbols;
	void *eof = malloc(sizeof(SYMBOL));
	NODE *root;
	CODIFICATION *codification;

	openFiles(&in, file_in, "rb", &out, file_out, "wb", &cod, file_cb, "rb");
	codification = fileToCode(cod, &symbols);
	memcpy(eof, codification[symbols-1].symbol, sizeof(SYMBOL));
	root = codeToTree(codification, symbols);
	huffmanDecode(out, in, root, eof);

	fclose(in);
	fclose(out);
	fclose(cod);
}
