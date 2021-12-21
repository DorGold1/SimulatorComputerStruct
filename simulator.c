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
    if (readData) {
        MEM[i] = -1;
    }
    else {
        cmdLst[i] = NULL;
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

void check_for_interrupt(int *triggers){
    triggers[0] = IORegister[0] & IORegister[3];
    triggers[1] = IORegister[1] & IORegister[4];
    triggers[2] = IORegister[2] & IORegister[5];
}


int add_to_data_lst(int *mem, char *data) {
    *mem = (int)strtol(data, NULL, 16);
}

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
		monitor_frame[i] = calloc(SECTOR_SIZE, sizeof(uint8_t));
    }
    int a=3;

    
int main_loop(){
    int triggers[3];
    int timer = 0;
    int disk_IO = 0;
    int Disk_cycle_timer;
    int type_of_operation;
	while(cmdLst[pc]){
        update_irq2(timer);
        check_for_interrupt(triggers);
        if((triggers[0] == 1) & (IN_INTERRUPT == 0)){
            IN_INTERRUPT = 1;
            IORegister[7] = pc; //SAVE ADDRESS FOR RETI
            pc = IORegister[6]; //GO TO INTERRUPT
            
            
            //do timer interrupt
        }
        if((triggers[1] == 1) & (IN_INTERRUPT == 0)){
            IN_INTERRUPT = 1;
            //do hard disk interrupt....
        }
         if((triggers[2] == 1) & (IN_INTERRUPT == 0)){
            IN_INTERRUPT = 1;
            //do irq2 interrupt..........
        }
        if((IORegister[14] != 0) & IORegister[17] == 0){ //NEED TO WRITE TO IO DEVICE
            Disk_cycle_timer = 0; //INITATE TIMER
            type_of_operation = IORegister[14];
            if(type_of_operation == 1){
                //READ FROM DEVICE...........
            }
            else{
                //WRITE TO DEVICE............
            }
        }
        ///////CHECK FOR TIMER INTERRUPT.......
        if(IORegister[11]){ //check if timer is enabled
            IORegister[12] += 1; //TIMER CURRENT += 1
        }
        if(IORegister[12] == IORegister[13]){
            IORegister[12] = 0; // TIMER CURRENT = 0
            IORegister[3] = 0; //turn on irq0 bit for timer.
        }
        ////////END CHECK FOR TIMER INTERRUPT
        int old_pc = pc; 
        run_command(cmdLst[pc]);//now pc has changed.......
        if(pc == old_pc){
            pc += 1;
        }
        
        timer+=1;
        if(Disk_cycle_timer == 1023){ // FREE IO DISK
            IORegister[4] = 1;
            IORegister[14] = 0;
            IORegister[17] = 0;
        }
        if(IORegister[17] == 1){
            Disk_cycle_timer += 1;
        }
    }
}


int run_command(Instruction *instruction){
    if(instruction->op <= 8){
        run_arithmetic(*instruction,instruction->op);
    }   
    else if(instruction->op <= 15){
        run_jump_branch_commands(*instruction , instruction->op);
    }
    else if(instruction->op <= 17){
        run_memory_command(*instruction,instruction->op);
    }
    else if(instruction->op <= 20){
        run_IOregister_operations(*instruction, instruction->op);
    }
    else if(instruction->op == 21){
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

void run_IOregister_operations(Instruction instruction , int id){
    if(id == 18){
        pc = IORegister[7];
        IN_INTERRUPT = 0; //SO FLAG WON'T BE TRIGGERED
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
