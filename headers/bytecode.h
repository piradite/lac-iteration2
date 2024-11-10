#ifndef BYTECODE_H
#define BYTECODE_H
#include <stdio.h>

typedef enum {
    PRINT_OP = 0x01,
    INT_OP = 0x02
} OpcodeEnum;

typedef enum {
    ERR_UNKNOWN_INSTRUCTION = -1,
    ERR_FILE_ERROR = -2,
    ERR_MALFORMED_PRINT = -3,
    ERR_UNINITIALIZED_VARIABLE = -4
} ErrorCode;

typedef void (*InstructionHandler)(const char *args, FILE * output);
typedef void (*OpcodeHandler)(FILE * input);

typedef struct {
    const char *name;
    unsigned char opcode;
    InstructionHandler handler;
} Instruction;

typedef struct {
    unsigned char opcode;
    OpcodeHandler handler;
} Opcode;

typedef struct {
    char name[32];
    int value;
} Variable;

extern Variable variables[100];
extern size_t variable_count;
extern Instruction instruction_set[];
extern size_t INSTRUCTION_COUNT;
extern Opcode opcode_set[];
extern size_t OPCODE_COUNT;

void handle_print(const char *args, FILE * output);
void handle_print_opcode(FILE * input);

void handle_int(const char *args, FILE * output);
void handle_int_opcode(FILE * input);

void handle_print(const char *args, FILE * output);
void handle_print_opcode(FILE * input);

int set_variable(const char *name, int value);
int get_variable(const char *name, int *value);

void compile(const char *input_file, const char *output_file);

void log_error(const char *message);
void log_message(const char *message);

#endif
