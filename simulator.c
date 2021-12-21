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

int read_instructions(FILE *fp, int len, Instruction **cmdLst);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int convert_str_to_num(int len, char *str);
char cut_string_by_index(char *str, int i);

int main() {
    int i;
    const char *filename = "imemin.txt";
    FILE *fp = fopen(filename,"r");
	Instruction **cmdLst = calloc(MAX_INSTRUCTIONS,sizeof(Instruction *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        cmdLst[i] = (Instruction *)malloc(sizeof(Instruction *));
    }
    read_instructions(fp,INSTRUCTION_LEN,cmdLst);
    int a=3;
      
}

int read_instructions(FILE *fp, int len, Instruction **cmdLst) {
    int i = 0;
    char *line = malloc(len*sizeof(char));
    while (fgets(line, len, fp)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_cmd_lst(cmdLst[i++], line);
    }
	return 1;
}

int add_to_cmd_lst(Instruction *cmdLst, char *inst) {
	char tmp;
    tmp = cut_string_by_index(inst, 2);	//OP
    cmdLst -> op = (short)strtol(inst, NULL, 16);
    inst[2]=tmp;
    tmp = cut_string_by_index(inst, 3);	//rd
    cmdLst -> rd = (short)strtol(inst+2, NULL, 16);
    inst[3]=tmp;
    tmp = cut_string_by_index(inst, 4);	//rs
    cmdLst -> rs = (short)strtol(inst+3, NULL, 16);
    inst[4]=tmp;
    tmp = cut_string_by_index(inst, 5);	//rt
    cmdLst -> rt = (short)strtol(inst+4, NULL, 16);
    inst[5]=tmp;
    tmp = cut_string_by_index(inst, 6);	//rm
    cmdLst -> rm = (short)strtol(inst+5, NULL, 16);
    inst[6]=tmp;
    tmp = cut_string_by_index(inst, 9);	//imm1
    cmdLst -> immediate1 = (short)strtol(inst+6, NULL, 16);
    inst[9]=tmp;						//imm2
    cmdLst -> immediate2 = (short)strtol(inst+9, NULL, 16);
    return 1;
}

char cut_string_by_index(char *str, int i) {
	char tmp = str[i];
    str[i] = '\0';
    return tmp;
}


