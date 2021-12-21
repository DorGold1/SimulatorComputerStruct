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
#define SUCCESS 0
#define FAILURE 1


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

int MEM[MAX_SIZE];

int R[16];

const char *inst_filename = "imemin.txt";
const char *data_filename = "dmemin.txt";


int read_from_file(FILE *fp, int len, bool readData);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int add_to_data_lst(int *mem, char *data);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int convert_str_to_num(int len, char *str);
