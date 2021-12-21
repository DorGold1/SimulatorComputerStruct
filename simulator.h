#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include "utils.c"


#define INSTRUCTION_LEN 13
#define DATA_LEN 9
#define MAX_SIZE 4096
#define MAX_INSTRUCTIONS 4096
#define NUM_OF_IOREGISTERS 23

int pc = 0;
int IORegister[NUM_OF_IOREGISTERS];
int MEM[MAX_SIZE];
int R[16];
const char *inst_filename = "imemin.txt";
const char *data_filename = "dmemin.txt";


typedef struct instruction {
    short op;
    short rd;
    short rs;
    short rt;
    short rm;
    int immediate1;
    int immediate2;
} Instruction;

Instruction **cmdLst;


int read_from_file(FILE *fp, int len, bool readData);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int add_to_data_lst(int *mem, char *data);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int run_command(Instruction instruction);
void run_arithmetic(Instruction instruction, int id);
void run_jump_branch_commands(Instruction instruction, int id);
void run_memory_command(Instruction instruction , int id);
void run_IOregister_operation(Instruction instruction , int id);
