#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/bytecode.h"

int set_variable(const char *name, int value) {
    for (size_t i = 0; i < variable_count; i++) {
	if (strcmp(variables[i].name, name) == 0) {
	    variables[i].value = value;
	    return 0;
	}
    }
    strncpy(variables[variable_count].name, name, 32);
    variables[variable_count].value = value;
    variable_count++;
    return 0;
}

int get_variable(const char *name, int *value) {
    for (size_t i = 0; i < variable_count; i++) {
	if (strcmp(variables[i].name, name) == 0) {
	    *value = variables[i].value;
	    return 0;
	}
    }
    return -1;
}
