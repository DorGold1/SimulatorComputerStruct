#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSTRUCTION_LEN 13
#define MAX_INSTRUCTIONS 4096

typedef struct instruction {
    short op;
    short rd;
    short rs;
    short rt;
    short rm;
    int immediate1;
    int immediate2;
} Instruction;

int R[16];

int read_instructions(FILE *fp, int len, Instruction **cmdLst);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int convert_str_to_num(int len, char *str);
char cut_string_by_index(char *str, int i);
