#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/bytecode.h"

void handle_int(const char *args, FILE *output) {
    fwrite(&(unsigned char) { INT_OP }, 1, 1, output);

    char name[32];
    int value;
    char *colon_ptr = strchr(args, ':');
    char *arrow_ptr = strstr(args, "->");

    if (!colon_ptr || !arrow_ptr) {
	fprintf(stderr, "Error: Invalid int assignment syntax\n");
	exit(ERR_MALFORMED_PRINT);
    }

    size_t name_len = colon_ptr - args;
    if (name_len >= sizeof(name)) {
	fprintf(stderr, "Error: Variable name is too long\n");
	exit(ERR_MALFORMED_PRINT);
    }
    strncpy(name, args, name_len);
    name[name_len] = '\0';

    if (sscanf(arrow_ptr + 2, "%d", &value) != 1) {
	fprintf(stderr, "Error: Invalid integer value in assignment\n");
	exit(ERR_MALFORMED_PRINT);
    }

    fwrite(&name_len, sizeof(size_t), 1, output);
    fwrite(name, sizeof(char), name_len, output);
    fwrite(&value, sizeof(int), 1, output);
    set_variable(name, value);
}

void handle_int_opcode(FILE *input) {
    size_t name_len;
    fread(&name_len, sizeof(size_t), 1, input);

    char name[32];
    fread(name, sizeof(char), name_len, input);
    name[name_len] = '\0';

    int value;
    fread(&value, sizeof(int), 1, input);

    set_variable(name, value);
}
