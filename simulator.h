#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*---------------------------------------------------------------Define Constants & Structs----------------------------------------------------*/
#define MAX_DATA 4096
#define REG_HEX_LEN 8
#define MAX_INSTRUCTIONS 4096
#define INSTRUCTION_LEN 13
#define REGISTERS_LEN 16
#define NUM_IOREGISTERS 23
#define NUM_SECTORS 128
#define SECTOR_SIZE 128
#define MONITOR_RES 256
#define TRACE_LEN 161
#define TXT_LEN 20
#define CONTINUE_MAINLOOP 2


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


/*---------------------------------------------------------------GLOBAL VARIABLES-------------------------------------------------------------*/
int R[REGISTERS_LEN];
int IORegister[NUM_IOREGISTERS];
char IORegNames[NUM_IOREGISTERS][NUM_IOREGISTERS] = {"irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status" , "irq2status", "irqhandler", "irqreturn", "clks" , "leds", "display7seg", "timerenable", "timercurrent", "timermax", "diskcmd", "disksector", "diskbuffer", "diskstatus", "NULL", "NULL", "monitoraddr", "monitordata", "monitorcmd"};
char **filenames, **instructions;
int MEM[MAX_DATA];
int dataMaxIndex = 0, diskMaxIndex = 0, PC = 0, cycles = 0, led = 0, irq2Index = 0, inInterrupt = 0, diskCycleTimer = 0;
int *diskIO, *monitorFrame, *irq2Lst;
int immMask = ~(-1 + (1<<12 -1));


/*---------------------------------------------------------------FUNCTIONS DECLAREATIONS------------------------------------------------------*/
//Handler Funcs
void add_to_cmd_lst(Instruction *cmdLst, char *inst);
void main_loop();
void interrupt_handler();
void update_irqs_state(int *irqState);
void diskIO_handler();
void timer_handler();
void update_monitor_pixels();
void update_irq2(int cycle);

//Run Command Funcs
int run_command(Instruction instruction);
void run_arithmetic(Instruction instruction, int id);
void run_jump_branch_commands(Instruction instruction, int id);
void run_memory_command(Instruction instruction , int id);
void run_IOregister_operation(Instruction instruction , int id);

//Txt Files Funcs
void init_txt_files();
void write_exit_txt_files();
