#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/bytecode.h"

int set_variable(const char *name, VariableType type, void *value) {
    for (size_t i = 0; i < variable_count; i++) {
	if (strcmp(variables[i].name, name) == 0) {
	    variables[i].type = type;
	    if (type == TYPE_INT) {
		variables[i].int_value = *(int *)value;
	    } else {
		variables[i].float_value = *(float *)value;
	    }
	    return 0;
	}
    }

    strncpy(variables[variable_count].name, name, 32);
    variables[variable_count].type = type;
    if (type == TYPE_INT) {
	variables[variable_count].int_value = *(int *)value;
    } else {
	variables[variable_count].float_value = *(float *)value;
    }
    variable_count++;
    return 0;
}

int get_variable(const char *name, Variable *var) {
    for (size_t i = 0; i < variable_count; i++) {
	if (strcmp(variables[i].name, name) == 0) {
	    *var = variables[i];
	    return 0;
	}
    }
    return -1;
}
