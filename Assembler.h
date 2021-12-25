#ifndef INCLUDEINIT
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#endif

/*---------------------------------------------------------------Define Constants-------------------------------------------------------------*/
#define memSize 4096
#define MAX_STR_LEN 100
#define MAX_LINE_LEN 8
#define conversionSize 38
#define dEntryLen 8
#define iEntryLen 12
#define Label_flag "LBF"

/*---------------------------------------------------------------FUNCTIONS & TYPEDEF DECLAREATIONS--------------------------------------------*/

void str2param (char* result, const char *str);
void str2Hex (char* result, char *str);
int isLabel(char *str); /*returns 1 if str is label*/
int isHexa (char* str); /*returns 0 if str isn't hex rep of number, otherwise returns its length*/
int str_to_2complement(char* str, int hex_len);  /*get number as a string and last idx of binary rep - returns it's value in 2's complement rep*/
void add_op_to_result(char* result, char* str);
void add_reg_to_result(char* result, char* str);
void add_imm_to_result(char* result, char* str);

typedef struct data_node data_node;
typedef struct Queue Queue;
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

/*---------------------------------------------------------------CONVERSION TABLE---------------------------------------------------------------*/
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


void str2param (char* result, const char *str){
     for (int i = 0;  i < conversionSize;  i++){
        if (strcmp(str, conversionparam[i].str)==0) { strcpy(result,conversionparam[i].param_code); break;}
    }
}

void str2Hex (char *result, char *str){
    //gets hexa num or signed dec num as str--> returns into result its hexa rep
    int isHex, hex_len = strlen(result);
    isHex = isHexa(str);
    if (isHex) {
        int startIdx = hex_len-isHex;
        strcpy(result+startIdx, str+2);
    }
    else {
        int two_com_rep = str_to_2complement(str, hex_len);
        dec2hexa(result, two_com_rep, hex_len);
    }
}

int add_imm_to_result(char* result, char* str){
    if (isLabel(str)) {
        strcat(result, Label_flag);
        return 1;
    }
    else {
        char *tmp = (char *) malloc(3*sizeof(char));
        for (int i=0; i< 3; i++){ tmp[i] = '0';}
        str2Hex(tmp, str);
        strcat(result, tmp);
        return 0;
    }
}

void add_op_to_result(char* result, char* str){
    char *tmp = (char *) calloc(2,sizeof(char));
    str2param(tmp, str);
    strcpy(result, tmp);
}

void add_reg_to_result(char* result, char* str){
    char *tmp = (char *) calloc(1,sizeof(char));
    str2param(tmp, str);
    strcat(result, tmp);
}

int str_to_2complement(char* str, int hex_len){
    //gets decimal num as str and returns it as int in 2's complement rep
    int num = strtol(str, (char**) NULL, 10);
    int max_bits_idx = (hex_len*4)-1;
    if (num>=0) { return num;}
    else {
        int max_neg = 1<<max_bits_idx;
        int delta = max_neg+num;
        return (max_neg | delta);
    }
}

int isHexa (char* str){// check 0xnull case ??
   if (str[0] == '0' && (str[1] == 'x' || str[1] = 'X')) {return strlen(str+2);}
   return 0;
}

