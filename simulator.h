#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>

#define INSTRUCTION_LEN 13
#define DATA_LEN 9
#define MAX_DATA 4096
#define MAX_INSTRUCTIONS 4096
#define NUM_OF_IOREGISTERS 23
#define NUM_SECTORS 128
#define SECTOR_SIZE 128
#define MONITOR_RES 256

typedef enum {data, instruction, irq2, disk} Mode;


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
int MEM[MAX_DATA];
int *irq2Lst;
int irq2Index = 0;
int **diskIO;
uint8_t **monitor_frame;
int pc = 0;
int IORegister[NUM_OF_IOREGISTERS];
int R[16];
const char *inst_filename = "imemin.txt";
const char *data_filename = "dmemin.txt";
const char *irq2_filename = "irq2in.txt";
const char *disk_filename = "diskin.txt";


//Func Declarations
int run_command(Instruction instruction);
void run_arithmetic(Instruction instruction, int id);
void run_jump_branch_commands(Instruction instruction, int id);
void run_memory_command(Instruction instruction , int id);
void run_IOregister_operation(Instruction instruction , int id);
void update_irq2(int cycle);


//Utils Func Declarations
int read_from_file(FILE *fp, int len, Mode mode);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int add_to_data_lst(int *mem, char *data);
int add_to_irq2_lst(int *irq2, char *data);
char cut_string_by_index(char *str, int i);
void fill_with_null(int start, int end, Mode mode);
int compare (const void * a, const void * b);