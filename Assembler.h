#ifndef INCLUDEINIT
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#endif

/*---------------------------------------------------------------Define Constants & Types----------------------------------------------------*/
#define MAX_ASSEMBLY_INSTRUCTIONS 10000000000
#define memSize 4096
#define MAX_STR_LEN 100
#define MAX_LINE_LEN 8
#define conversionSize 38
#define dEntryLen 8
#define iEntryLen 12
#define Label_flag "LBF"

typedef struct data_node data_node;
typedef struct Queue Queue;

typedef enum {
    LabelOnly,
    InstructionOnly,
    WordOnly,
    Label_Instruction,
    Empty
} lineType;

typedef enum {
    reg,
    op,
    imm
} paramType;

/*---------------------------------------------------------------GLOBAL DATA STRUCTURES-------------------------------------------------------*/

char **imem_table, **dmem_table, **unparsed_instructions;
Queue *Labels, *instructions_with_label;
/*---------------------------------------------------------------FUNCTIONS & TYPEDEF DECLAREATIONS--------------------------------------------*/

void str2param (char* result, const char *str);
void str2Hex (char* result, char *str);
int isLabel(char *str); /*returns 1 if str is label*/
int isHexa (char* str); /*returns 0 if str isn't hex rep of number, otherwise returns its length*/
int str_to_2complement(char* str, int hex_len);  /*get number as a string and last idx of binary rep - returns it's value in 2's complement rep*/
void add_op_to_result(char* result, char* str);
void add_reg_to_result(char* result, char* str);
int add_imm_to_result(char* result, char* str);

void init_DS();

/*gets line as read from file and getting it ready for parsing*/
int fix_line_for_parsing(char** broken_res, char *lineBuffer);
/*Helping Functions*/
int break_buffer(char **broken_buffer, char *lineBuffer);
void drop_comment(char **line, int line_len);
/*end of helping functions*/

/*gets a fixed line from asm file as line and adds it parsed to relevant table*/
void parseLine(char **line, int lLength, int *pc);
/*Helping Functions*/
void parseInstruction(char** line, int pc);
void parseLabel(char *label_name, int pc);
void parseWord (char **line);

void add_line_to_table(char *parsedLine, int hex_address, int is_intruction);
void update_pc(int *pc, lineType lt);
/*end of helping functions*/

/*returns the type of line currently being read*/
lineType get_lineType(int line_len);

/*Queue - Helping Functions*/
void add_to_Queue(data_node *new, int isLabel);
void make_new_node(data_node *new, char *name, int pc);
int get_label_pc(char* search_term);
int label_exist(char *label_name);
/*end of helping functions*/

void update_labled_instructions();


/*---------------------------------------------------------------ENUMS & STRUCTS---------------------------------------------------------------*/

struct data_node{
    char *name;
    int pc_num;
    data_node* next;
};

struct Queue{
    data_node *head;
    data_node *tail;
};

//CONVERSION TABLE
const static struct {//used for op/reg
    const char *param_code;
    const char *str;
} conversionparam [] = {
    {"00", "add"},
    {"01", "sub"},
    {"02", "mac"},
    {"03", "and"},
    {"04", "or"},
    {"05", "xor"},
    {"06", "sll"},
    {"07", "sra"},
    {"08", "srl"},
    {"09", "beq"},
    {"0A", "bne"},
    {"0B", "blt"},
    {"0C", "bgt"},
    {"0D", "ble"},
    {"0E", "bge"},
    {"0F", "jal"},
    {"10", "lw"},
    {"11", "sw"},
    {"12", "reti"},
    {"13", "in"},
    {"14", "out"},
    {"15", "halt"},
    {"0", "$zero"},
    {"1", "$imm1"},
    {"2", "$imm2"},
    {"3", "$v0"},
    {"4", "$a0"},
    {"5", "$a1"},
    {"6", "$a2"},
    {"7", "$t0"},
    {"8", "$t1"},
    {"9", "$t2"},
    {"A", "$s0"},
    {"B", "$s1"},
    {"C", "$s2"},
    {"D", "$gp"},
    {"E", "$sp"},
    {"F", "$ra"},
};


