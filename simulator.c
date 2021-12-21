#include "simulator.h"
#include <math.h>

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

void run_arithmetic(Instruction instruction, int id){
    if(id == 0){
        R[instruction.rd] = R[instruction.rs] + R[instruction.rt] + R[instruction.rm];
    }
    else if(id == 1){
        R[instruction.rd] = R[instruction.rs] - R[instruction.rt] - R[instruction.rm];
    }
    else if(id == 2){
        R[instruction.rd] = R[instruction.rs] * R[instruction.rt] + R[instruction.rm];
    }
    else if(id == 3){
        R[instruction.rd] = R[instruction.rs] & R[instruction.rt] & R[instruction.rm];
    }
    else if(id == 4){
        R[instruction.rd] = R[instruction.rs] | R[instruction.rt] | R[instruction.rm];
    }
    else if(id == 5){
        R[instruction.rd] = R[instruction.rs] ^ R[instruction.rt] ^ R[instruction.rm];
    }
    else if(id == 6){
        R[instruction.rd] = R[instruction.rs] << R[instruction.rt];
    }
    //check if this is ok!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    else if(id == 7){
        R[instruction.rd] = R[instruction.rs] / power(2,R[instruction.rt]); //need to do left arithmetic shift 
    }
    else if(id == 8){
        R[instruction.rd] = R[instruction.rs] >> R[instruction.rt];
    }

}

void run_jump_brunch_commands(Instruction instruction, int id){
    int mask = 4095;
    int program_counter_new_address = R[instruction.rm] & mask;
    if(id == 9){
        if(R[instruction.rs] == R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 10){
        if(R[instruction.rs] != R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 11){
        if(R[instruction.rs] < R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 12){
        if(R[instruction.rs] > R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 13){
        if(R[instruction.rs] <= R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 14){
        if(R[instruction.rs] >= R[instruction.rt]){
            pc = program_counter_new_address;
        }
    }
    else if(id == 15){
        R[instruction.rd] = pc + 1;
        pc = program_counter_new_address;
    }
}

void run_memmory_command(Instruction instruction , int id){
    if(id == 16){
        R[instruction.rd] = MEM[R[instruction.rs] + R[instruction.rt]] + R[instruction.rm]; 
    }
    else if(id == 17){
        MEM[R[instruction.rs] + R[instruction.rt]] = R[instruction.rm] + R[instruction.rt];
    }
}

void run_register_operation(Instruction instruction , int id){
    if(id == 18){
        pc = IORegister[7];
    }
    else if(id == 19){
        R[instruction.rd] = IORegister[R[instruction.rs] + R[instruction.rt]];
    }
    else if(id == 20){
        IORegister[R[instruction.rs]+R[instruction.rt]] = R[instruction.rm];
    }
}

int run_command(Instruction instruction){
    if(instruction.op <= 8){
        run_arithmetic(instruction,instruction.op);
    }
    else if(instruction.op <= 15){
        run_jump_brunch_commands(instruction , instruction.op);
    }
    else if(instruction.op <= 17){
        run_memmory_command(instruction,instruction.op);
    }
    else if(instruction.op <= 20){
        run_register_operations(instruction, instruction.op);
    }
    else if(instruction.op == 21){
        exit(1); //determine exit message
    }
    else{
        printf("op code not recognized"); //determine message
    }
}

int main_loop(){

}
