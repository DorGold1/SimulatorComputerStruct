#include "simulator.h"

int main() {
    int i, res;
    FILE *fp;
    fp = fopen(inst_filename,"r");
	cmdLst = calloc(MAX_SIZE,sizeof(Instruction *));
    for(i=0; i<MAX_SIZE; i++) {
        cmdLst[i] = (Instruction *)malloc(sizeof(Instruction *));
    }
    read_from_file(fp, INSTRUCTION_LEN, false);
	fclose(fp);
    fp = fopen(data_filename,"r");
	read_from_file(fp, DATA_LEN, true);
    fclose(fp);
    int a=3;
}

int read_from_file(FILE *fp, int len, bool readData) {
    int i = 0;
    char *line = malloc(len*sizeof(char));
    while(fgets(line, len, fp)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        if (readData) {
			add_to_data_lst(&MEM[i++], line);
        }
		else {
            add_to_cmd_lst(cmdLst[i++], line);
        }
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


int add_to_data_lst(int *mem, char *data) {
    *mem = (int)strtol(data, NULL, 16);
}


void run_add_or_sub(Instruction instruction, int id){
    int a = 1;
}

int run_command(Instruction instruction){
    if(instruction.op == 0 || instruction.op == 1){
        run_add_or_sub(instruction,instruction.op);
    }
    

}
