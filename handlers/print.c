#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "../headers/bytecode.h"

void handle_print(const char *args, FILE *output) {
    fwrite(&(unsigned char) { PRINT_OP }, 1, 1, output);
    char *final_output = NULL;
    size_t total_length = 0;
    const char *segment = args;
    int expect_sep = 0;

    while (*segment) {
        while (isspace((unsigned char)*segment)) segment++;
        if (expect_sep) {
		int nl_count = 0;
		while (*segment == '.') nl_count++, segment++;

		if (nl_count > 0) {
		char *temp_output = realloc(final_output, total_length + nl_count + 1);
		if (!temp_output) { perror("Memory allocation failed"); free(final_output); exit(1); }
		final_output = temp_output;
		memset(final_output + total_length, '\n', nl_count);
		total_length += nl_count;
		final_output[total_length] = '\0';
		expect_sep = 0;
		continue;
		} else if (*segment == ',') { segment++; expect_sep = 0; continue; }
		else { fprintf(stderr, "Error: Expected ',' or '.'\n"); free(final_output); exit(ERR_MALFORMED_PRINT); }
        }

        int print_address = 0;
        if (*segment == '&') {
		print_address = 1;
		segment++;
        }

        if (isalpha(*segment) || *segment == '_') {
		Variable var;
		char name[32];
		int index = -1;

		const char *start = segment;
		while (isalnum(*segment) || *segment == '_') segment++;
		size_t name_len = segment - start;
		if (name_len >= sizeof(name)) { fprintf(stderr, "Variable name too long\n"); free(final_output); exit(ERR_MALFORMED_PRINT); }
		strncpy(name, start, name_len);
		name[name_len] = '\0';

		if (*segment == '%') {
		segment++;
		if (isdigit(*segment)) { index = atoi(segment); while (isdigit(*segment)) segment++; }
		else { fprintf(stderr, "Error: Invalid list index format\n"); free(final_output); exit(ERR_MALFORMED_PRINT); }
		}

		if (get_variable(name, &var) == -1) { fprintf(stderr, "Uninitialized variable '%s'\n", name); free(final_output); exit(ERR_UNINITIALIZED_VARIABLE); }

		char value_str[64] = { 0 };
		int value_len = 0;

		if (print_address) {
		if (index >= 0 && var.type == TYPE_LIST) {
		if (index >= var.list_value->count) { fprintf(stderr, "Index %d out of bounds for list '%s'\n", index, name); free(final_output); exit(ERR_OUT_OF_BOUNDS); }
		Variable *elem = var.list_value->elements[index - 1];
		snprintf(value_str, sizeof(value_str), "%p", (void *)elem);
		} else {
		snprintf(value_str, sizeof(value_str), "%p", (void *)&var);
		}
		value_len = strlen(value_str);

		} else if (index >= 0 && var.type == TYPE_LIST) {
		if (index >= var.list_value->count) { fprintf(stderr, "Index %d out of bounds for list '%s'\n", index, name); free(final_output); exit(ERR_OUT_OF_BOUNDS); }
		Variable *elem = var.list_value->elements[index - 1];
		switch (elem->type) {
		case TYPE_INT: snprintf(value_str, sizeof(value_str), "%lld", elem->int_value); break;
		case TYPE_FLOAT: snprintf(value_str, sizeof(value_str), "%.2f", elem->float_value); break;
		case TYPE_STRING: snprintf(value_str, sizeof(value_str), "%s", elem->string_value); break;
		case TYPE_CHAR: snprintf(value_str, sizeof(value_str), "%c", elem->char_value); break;
		case TYPE_BOOL: snprintf(value_str, sizeof(value_str), "%s", elem->bool_value ? "true" : "false"); break;
		default: snprintf(value_str, sizeof(value_str), "Unknown");
		}

        value_len = strlen(value_str);
        } else {
        switch (var.type) {
        case TYPE_INT: snprintf(value_str, sizeof(value_str), "%lld", var.int_value); value_len = strlen(value_str); break;
        case TYPE_FLOAT: {
        if (index >= 0) {
        snprintf(value_str, sizeof(value_str), "%.*f", index, var.float_value);
        } else {
        snprintf(value_str, sizeof(value_str), "%g", var.float_value);
        }
		value_len = strlen(value_str);
		break;
        }
		case TYPE_STRING: {
		value_len = strlen(var.string_value);
		char *temp_output = realloc(final_output, total_length + value_len + 1);
		if (!temp_output) { perror("Memory allocation failed"); free(final_output); exit(1); }
		final_output = temp_output;
		strcpy(final_output + total_length, var.string_value);
		total_length += value_len;
		final_output[total_length] = '\0';
		expect_sep = 1;
		continue;
		}
		case TYPE_CHAR: snprintf(value_str, sizeof(value_str), "%c", var.char_value); value_len = 1; break;
		case TYPE_BOOL: snprintf(value_str, sizeof(value_str), "%s", var.bool_value ? "true" : "false"); value_len = strlen(value_str); break;
		case TYPE_LIST: {
		List *list = var.list_value;
		for (size_t i = 0; i < list->count; i++) {
		Variable *elem = list->elements[i];
		char elem_str[64] = { 0 };

		switch (elem->type) {
		case TYPE_INT: snprintf(elem_str, sizeof(elem_str), "%lld", elem->int_value); break;
		case TYPE_FLOAT: snprintf(elem_str, sizeof(elem_str), "%.2f", elem->float_value); break;
		case TYPE_STRING: snprintf(elem_str, sizeof(elem_str), "%s", elem->string_value); break;
		case TYPE_CHAR: snprintf(elem_str, sizeof(elem_str), "%c", elem->char_value); break;
		case TYPE_BOOL: snprintf(elem_str, sizeof(elem_str), "%s", elem->bool_value ? "true" : "false"); break;
		default: snprintf(elem_str, sizeof(elem_str), "Unknown");
		}

		size_t elem_len = strlen(elem_str);
		char *temp_output = realloc(final_output, total_length + elem_len + 3);
		if (!temp_output) { perror("Memory allocation failed"); free(final_output); exit(1); }

		final_output = temp_output;
		strcpy(final_output + total_length, elem_str);
		total_length += elem_len;

		if (i < list->count - 1) {
			final_output[total_length++] = ',';
			final_output[total_length++] = ' ';
		}
		final_output[total_length] = '\0';
		}
		break;
		}
		default: fprintf(stderr, "Unknown variable type\n"); free(final_output); exit(ERR_MALFORMED_PRINT);
        }
		}

		char *temp_output = realloc(final_output, total_length + value_len + 1);
		if (!temp_output) { perror("Memory allocation failed"); free(final_output); exit(1); }
		final_output = temp_output;
		strncpy(final_output + total_length, value_str, value_len);
		total_length += value_len;
		final_output[total_length] = '\0';

        expect_sep = 1;
        } else {
		fprintf(stderr, "Unexpected character '%c'\n", *segment);
		free(final_output);
		exit(ERR_MALFORMED_PRINT);
        }
    	}

		fwrite(&total_length, sizeof(size_t), 1, output);
		fwrite(final_output, sizeof(char), total_length, output);
		free(final_output);
}


void handle_print_opcode(FILE *input) {
    size_t length;
    if (fread(&length, sizeof(size_t), 1, input) != 1) {
	fprintf(stderr, "Error reading print string length\n");
	exit(ERR_FILE_ERROR);
    }

    if (length > 10000) {
	fprintf(stderr, "Error: Print string length is unreasonably large (%zu)\n", length);
	exit(ERR_MALFORMED_PRINT);
    }

    char *text = malloc(length + 1);
    if (!text) { perror("Memory allocation failed"); exit(1); }

    if (fread(text, 1, length, input) != length) {
	fprintf(stderr, "Error reading print string data\n");
	free(text);
	exit(ERR_FILE_ERROR);
    }
    text[length] = '\0';
    printf("%s\n", text);
    free(text);
}
