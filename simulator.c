#include "simulator.h"
#include "utils.c"


int main() {
    int i, res;
    FILE *fp;
    Mode mode;

    mode = instruction;
    fp = fopen(inst_filename,"r");
	cmdLst = malloc(MAX_INSTRUCTIONS * sizeof(Instruction *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        cmdLst[i] = (Instruction *)malloc(sizeof(Instruction *));
    }
    read_from_file(fp, INSTRUCTION_LEN, mode);
	fclose(fp);

    mode = data;
    fp = fopen(data_filename,"r");
	read_from_file(fp, DATA_LEN, mode);
    fclose(fp);

	mode = irq2;
    irq2Lst = calloc(MAX_INSTRUCTIONS, sizeof(int));
    fp = fopen(irq2_filename,"r");
    read_from_file(fp, INSTRUCTION_LEN, mode);
    fclose(fp);

	mode = disk;
    fp = fopen(disk_filename,"r");
    diskIO = malloc(NUM_SECTORS * sizeof(int *));
    for (i=0; i<NUM_SECTORS; i++) {
		diskIO[i] = calloc(SECTOR_SIZE, sizeof(int));
    }
	//READ DISK - HOW?!?!?!?

    //Init the rest of the IO devices
    monitor_frame = malloc(MONITOR_RES * sizeof(uint8_t *));
    for (i=0; i<MONITOR_RES; i++) {
		monitor_frame[i] = calloc(MONITOR_RES, sizeof(uint8_t));
    }
    int a=3;
}


int run_command(Instruction instruction){
    if(instruction.op <= 8){
        run_arithmetic(instruction,instruction.op);
    }
    else if(instruction.op <= 15){
        run_jump_branch_commands(instruction , instruction.op);
    }
    else if(instruction.op <= 17){
        run_memory_command(instruction,instruction.op);
    }
    else if(instruction.op <= 20){
        run_IOregister_operation(instruction, instruction.op);
    }
    else if(instruction.op == 21){
        exit(1); //determine exit message
    }
    else{
        fprintf(stderr, "op code not recognized"); //determine message
    }
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
    else if(id == 7){
        R[instruction.rd] = R[instruction.rs] >> R[instruction.rt]; //need to do right arithmetic shift 
    }
    else if(id == 8){
        R[instruction.rd] =(int) ((unsigned int)R[instruction.rs] >> R[instruction.rt]);
    }
}


void run_jump_branch_commands(Instruction instruction, int id){
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


void run_memory_command(Instruction instruction , int id){
    if(id == 16){
        R[instruction.rd] = MEM[R[instruction.rs] + R[instruction.rt]] + R[instruction.rm]; 
    }
    else if(id == 17){
        MEM[R[instruction.rs] + R[instruction.rt]] = R[instruction.rm] + R[instruction.rt];
    }
}


void run_IOregister_operation(Instruction instruction , int id){
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


void update_irq2(int cycle) {
    if (irq2Lst[irq2Index] == cycle) {
        IORegister[5] = 1;
        irq2Index++;
    }
}
