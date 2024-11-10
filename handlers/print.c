#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../headers/bytecode.h"

void handle_print(const char *args, FILE *output) {
    fwrite(&(unsigned char) { PRINT_OP }, 1, 1, output);

    size_t total_length = 0;
    char *final_output = NULL;

    const char *segment = args;
    int expecting_separator = 0;

    while (*segment) {
	if (expecting_separator) {
	    int newline_count = 0;

	    while (*segment == '.') {
		newline_count++;
		segment++;
	    }

	    if (newline_count > 0) {
		final_output = realloc(final_output, total_length + newline_count + 1);
		if (!final_output) {
		    fprintf(stderr, "Error: Memory allocation failed\n");
		    exit(1);
		}

		for (int i = 0; i < newline_count; i++) {
		    final_output[total_length++] = '\n';
		}
		final_output[total_length] = '\0';
		expecting_separator = 0;
		continue;
	    } else if (*segment == ',') {
		segment++;
		expecting_separator = 0;
		continue;
	    } else {
		fprintf(stderr, "Error: Expected ',' or '.' between print segments\n");
		exit(ERR_MALFORMED_PRINT);
	    }
	}

	if (*segment == '"' || *segment == '\'') {
	    char quote_char = *segment++;
	    const char *start = segment;
	    const char *end = strchr(segment, quote_char);

	    if (!end) {
		fprintf(stderr, "Error: Unmatched quotation in print statement\n");
		exit(ERR_MALFORMED_PRINT);
	    }

	    size_t length = end - start;
	    final_output = realloc(final_output, total_length + length + 1);
	    if (!final_output) {
		fprintf(stderr, "Error: Memory allocation failed\n");
		exit(1);
	    }

	    strncpy(final_output + total_length, start, length);
	    total_length += length;
	    final_output[total_length] = '\0';
	    segment = end + 1;
	    expecting_separator = 1;
	} else if (isalpha(*segment) || *segment == '_') {
	    const char *start = segment;
	    while (isalnum(*segment) || *segment == '_')
		segment++;

	    char name[32];
	    size_t name_length = segment - start;
	    if (name_length >= sizeof(name)) {
		fprintf(stderr, "Error: Variable name too long\n");
		exit(ERR_MALFORMED_PRINT);
	    }
	    strncpy(name, start, name_length);
	    name[name_length] = '\0';

	    int value;
	    if (get_variable(name, &value) == -1) {
		fprintf(stderr, "Error: Uninitialized variable '%s'\n", name);
		exit(ERR_UNINITIALIZED_VARIABLE);
	    }

	    char value_str[12];
	    int value_length = snprintf(value_str, sizeof(value_str), "%d", value);
	    final_output = realloc(final_output, total_length + value_length + 1);
	    if (!final_output) {
		fprintf(stderr, "Error: Memory allocation failed\n");
		exit(1);
	    }

	    strncpy(final_output + total_length, value_str, value_length);
	    total_length += value_length;
	    final_output[total_length] = '\0';
	    expecting_separator = 1;
	} else {
	    segment++;
	}
    }

    fwrite(&total_length, sizeof(size_t), 1, output);
    fwrite(final_output, sizeof(char), total_length, output);
    free(final_output);
}

void handle_print_opcode(FILE *input) {
    size_t length;
    fread(&length, sizeof(size_t), 1, input);

    char *text = malloc(length + 1);
    if (!text) {
	fprintf(stderr, "Error: Memory allocation failed for text\n");
	exit(1);
    }

    if (length == sizeof(int)) {
	int value;
	fread(&value, sizeof(int), 1, input);
	printf("%d\n", value);
    } else {
	fread(text, sizeof(char), length, input);
	text[length] = '\0';
	printf("%s\n", text);
    }

    free(text);
}
