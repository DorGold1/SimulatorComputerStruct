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
}Instruction;


typedef struct argument_registers {
    int a0;
    int a1;
    int a2;
} ArgRegs;

typedef struct temporary_registers {
    int t0;
    int t1;
    int t2;
} TmpRegs;

typedef struct saved_registers {
    int s0;
    int s1;
    int s2;
} SavedRegs;

typedef struct registers {
    ArgRegs arg;
    TmpRegs tmp;
    SavedRegs saved;
    int zero;
    int imm1;
    int imm2;
    int v0;
    int gp;
    int sp;
    int ra;
} Registers;

int read_instructions(FILE *fp, int len, Instruction **cmdLst);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int convert_str_to_num(int len, char *str);
char cut_string_by_index(char *str, int i);
