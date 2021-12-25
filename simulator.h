#define INCLUDEINIT
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>

#define INSTRUCTION_LEN 13
#define DATA_LEN 9
#define MAX_DATA 4096
#define MAX_INSTRUCTIONS 4096
#define NUM_IOREGISTERS 23
#define NUM_SECTORS 128
#define SECTOR_SIZE 128
#define MONITOR_RES 256
#define REGISTERS_LEN 16
#define TRACE_LEN 166


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

int immMask = ~(-1 + (1<<12 -1));

//Registers, Memory, IO Devices
int R[REGISTERS_LEN];
int IORegister[NUM_IOREGISTERS];
int MEM[MAX_DATA];
int PC = 0;
int **diskIO;
char **instructions;
uint8_t *monitorFrame;
int *irq2Lst;
int irq2Index = 0;
int inInterrupt = 0;
char ** filenames;

//cycle timer for disk
int diskCycleTimer = 0;

//Func Declarations
int main_loop();
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
void update_irq2(int cycle);
void update_irqs_state(int *irqState);
void interrupt_handler();
void diskIO_handler();
void timer_handler();
int run_command(Instruction instruction);
void run_arithmetic(Instruction instruction, int id);
void run_jump_branch_commands(Instruction instruction, int id);
void run_memory_command(Instruction instruction , int id);
void run_IOregister_operation(Instruction instruction , int id);
void write_from_disk(int* disk_sector, int* mem_buffer);
void read_from_disk(int* disk_sector, int* mem_buffer);
