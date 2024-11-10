#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../headers/bytecode.h"

void handle_print(const char *args, FILE *output) {
    fwrite(&(unsigned char) { PRINT_OP }, 1, 1, output);

    char *final_output = NULL;
    size_t total_length = 0;
    const char *segment = args;
    int expect_sep = 0;

    while (*segment) {
	if (expect_sep) {
		int nl_count = 0;
		while (*segment == '.') { nl_count++; segment++; }

		if (nl_count > 0) {
			final_output = realloc(final_output, total_length + nl_count + 1);
			if (!final_output) { perror("Memory allocation failed"); exit(1); }
			memset(final_output + total_length, '\n', nl_count);
			total_length += nl_count;
			final_output[total_length] = '\0';
			expect_sep = 0;
			continue;
		} else if (*segment == ',') { segment++; expect_sep = 0; continue; }
		else { fprintf(stderr, "Error: Expected ',' or '.'\n"); exit(ERR_MALFORMED_PRINT); }
	}

	if (*segment == '"' || *segment == '\'') {
		char quote = *segment++;
		const char *start = segment, *end = strchr(segment, quote);
		if (!end) { fprintf(stderr, "Unmatched quotation\n"); exit(ERR_MALFORMED_PRINT); }

		size_t len = end - start;
		final_output = realloc(final_output, total_length + len + 1);
		if (!final_output) { perror("Memory allocation failed"); exit(1); }

		strncpy(final_output + total_length, start, len);
		total_length += len;
		final_output[total_length] = '\0';
		segment = end + 1;
		expect_sep = 1;
	} else if (isalpha(*segment) || *segment == '_') {
		Variable var; char name[32]; int prec = -1;
		const char *start = segment;
		while (isalnum(*segment) || *segment == '_') segment++;

		size_t name_len = segment - start;
		if (name_len >= sizeof(name)) { fprintf(stderr, "Variable name too long\n"); exit(ERR_MALFORMED_PRINT); }
		strncpy(name, start, name_len); name[name_len] = '\0';

		if (get_variable(name, &var) == -1) { fprintf(stderr, "Uninitialized variable '%s'\n", name); exit(ERR_UNINITIALIZED_VARIABLE); }
		if (var.type == TYPE_FLOAT && *segment == '%') { segment++; if (isdigit(*segment)) prec = atoi(segment); while (isdigit(*segment)) segment++; }

		char value_str[32];
		if (var.type == TYPE_INT) snprintf(value_str, sizeof(value_str), "%d", var.int_value);
		else if (var.type == TYPE_FLOAT) snprintf(value_str, sizeof(value_str), (prec >= 0) ? "%.*f" : "%g", (prec >= 0) ? prec : 0, var.float_value);

		int value_len = strlen(value_str);
		final_output = realloc(final_output, total_length + value_len + 1);
		if (!final_output) { perror("Memory allocation failed"); exit(1); }

		strcpy(final_output + total_length, value_str);
		total_length += value_len;
		final_output[total_length] = '\0';
		expect_sep = 1;
	} else segment++;
	}

    fwrite(&total_length, sizeof(size_t), 1, output);
    fwrite(final_output, 1, total_length, output);
    free(final_output);
}

void handle_print_opcode(FILE *input) {
    size_t length;
    fread(&length, sizeof(size_t), 1, input);
    char *text = malloc(length + 1);
    if (!text) { perror("Memory allocation failed"); exit(1); }
    fread(text, 1, length, input);
    text[length] = '\0';
    printf("%s\n", text);
    free(text);
}
