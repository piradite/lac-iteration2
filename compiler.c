#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/bytecode.h"

Instruction instruction_set[] = {
    { "print", PRINT_OP, handle_print },
    { "int", INT_OP, handle_int },
    { "float", FLOAT_OP, handle_float }
};

size_t INSTRUCTION_COUNT = sizeof(instruction_set) / sizeof(Instruction);

Variable variables[100];
size_t variable_count;

void compile(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "wb");
    if (!input || !output) {
	log_error("File error");
	exit(ERR_FILE_ERROR);
    }

    char line[256];
    int in_block_comment = 0;

    while (fgets(line, sizeof(line), input)) {
	char *trimmed_line = line + strspn(line, " \t");
	trimmed_line[strcspn(trimmed_line, "\n")] = '\0';

	if (strlen(trimmed_line) == 0) {
	    continue;
	}
	if (in_block_comment) {
	    if (strstr(trimmed_line, "*;")) {
		in_block_comment = 0;
	    }
	    continue;
	}
	char *start_block_comment = strstr(trimmed_line, ";*");
	char *end_block_comment = strstr(trimmed_line, "*;");
	if (start_block_comment && !end_block_comment) {
	    in_block_comment = 1;
	    *start_block_comment = '\0';
	} else if (start_block_comment && end_block_comment) {
	    memmove(start_block_comment, end_block_comment + 2, strlen(end_block_comment + 2) + 1);
	}
	char *inline_comment = strstr(trimmed_line, ";;");
	if (inline_comment) {
	    *inline_comment = '\0';
	}
	trimmed_line = line + strspn(line, " \t");
	trimmed_line[strcspn(trimmed_line, "\n")] = '\0';
	if (strlen(trimmed_line) == 0) {
	    continue;
	}
	if (strstr(trimmed_line, ": int ->")) {
	    handle_int(trimmed_line, output);
	    continue;
	} else if (strstr(trimmed_line, ": float ->")) {
	    handle_float(trimmed_line, output);
	    continue;
	}

	char *command = strtok(trimmed_line, " ");
	char *args = strtok(NULL, "\n");

	int found = 0;
	for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
	    if (command && strcmp(command, instruction_set[i].name) == 0) {
		instruction_set[i].handler(args, output);
		found = 1;
		break;
	    }
	}
	if (!found) {
	    char error_msg[256];
	    snprintf(error_msg, sizeof(error_msg), "Unknown instruction '%s'", command ? command : "NULL");
	    log_error(error_msg);
	    exit(ERR_UNKNOWN_INSTRUCTION);
	}
    }

    fclose(input);
    fclose(output);
}
