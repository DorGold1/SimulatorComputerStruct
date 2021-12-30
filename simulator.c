#include "utils.c"

int main(int argc, char **argv) {
    
    int i, res;
    FILE *fp;
    Mode mode;
    filenames = argv;
    //Create files
    fp = fopen(filenames[7],"w");
    fclose(fp);
    fp = fopen(filenames[8],"w");
    fclose(fp);
    fp = fopen(filenames[10],"w");
    fclose(fp);
    fp = fopen(filenames[11],"w");
    fclose(fp);

    mode = instruction;
    fp = fopen(filenames[1],"r");
    instructions = malloc(MAX_INSTRUCTIONS * sizeof(char *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        instructions[i] = malloc(INSTRUCTION_LEN * sizeof(char));
    }
    read_from_file(fp, INSTRUCTION_LEN, mode);
	cmdLst = malloc(MAX_INSTRUCTIONS * sizeof(Instruction *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        cmdLst[i] = (Instruction *)malloc(sizeof(Instruction *));
        add_to_cmd_lst(cmdLst[i], instructions[i]);
    }
	fclose(fp);

    mode = data;
    fp = fopen(filenames[2],"r");
	read_from_file(fp, DATA_LEN+1, mode);
    fclose(fp);

	mode = disk;
    diskIO = calloc(NUM_SECTORS * SECTOR_SIZE ,sizeof(int));
    fp = fopen(filenames[3],"r");
	read_from_file(fp, NUM_SECTORS * SECTOR_SIZE, disk);
    fclose(fp);

	mode = irq2;
    irq2Lst = calloc(MAX_INSTRUCTIONS, sizeof(int));
    fp = fopen(filenames[4],"r");
    read_from_file(fp, INSTRUCTION_LEN, mode);
    fclose(fp);

    //Init the rest of the IO devices
    monitorFrame = calloc(MONITOR_RES * MONITOR_RES ,sizeof(int));
    for (i=0; i<MONITOR_RES * MONITOR_RES; i++) {
        monitorFrame[i]=0;
    }
    res = main_loop();
}


int add_to_cmd_lst(Instruction *cmdLst, char *inst) {
    if (inst == NULL) {
        cmdLst = NULL;
        return 1;
    }
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
    sign_ext(&(cmdLst -> immediate1));
    inst[9]=tmp;						//imm2
    cmdLst -> immediate2 = (short)strtol(inst+9, NULL, 16);
    sign_ext(&(cmdLst -> immediate2));
    return 1;
}


void update_monitor_pixels(){
    if(IORegister[22] == 1){
        int frame_data = IORegister[21];
        int frame_address = IORegister[20];
        monitorFrame[frame_address] = frame_data;
        IORegister[22] == 0;
    }
}


int main_loop() {
    int oldPC;
	while(cmdLst[PC]){
        //Handle disk and timer
        //Handle interrupts
        interrupt_handler();
        diskIO_handler();
        timer_handler();
        update_irq2(cycles);
        update_monitor_pixels();
        //Execute current command
        oldPC = PC; 
        run_command(*cmdLst[PC]);
        if(PC == oldPC) { //If PC has changed with a branch jump command - it doesn't need to be increased by 1.
            PC += 1;
        }
        //Update cycle count
        cycles+=1;
        IORegister[8] = cycles;
    }
    return EXIT_SUCCESS;
}


void update_irq2(int cycle) {
    if (irq2Lst[irq2Index] == cycle) {
        IORegister[5] = 1;
        irq2Index++;
    }
}


void interrupt_handler() {
    int irqState[3];
    update_irqs_state(irqState);
    if((irqState[0] == 1) && (inInterrupt == 0)) { //TIMER INTERRUPT
        inInterrupt = 1;
        IORegister[7] = PC; //SAVE ADDRESS FOR RETI
        PC = IORegister[6]; //GO TO INTERRUPT - PC = IRQHANDLER
    }
    if((irqState[1] == 1) && (inInterrupt == 0)) { //DISK IO INTERRUPT
        inInterrupt = 1;
        IORegister[7] = PC; //SAVE ADDRESS FOR RETI
        PC = IORegister[6]; //GO TO INTERRUPT - PC = IRQHANDLER
    }
    if((irqState[2] == 1) && (inInterrupt == 0)) { //RETI 2 INTERRUOT
        inInterrupt = 1;
        IORegister[7] = PC; //SAVE ADDRESS FOR RETI
        PC = IORegister[6]; //GO TO INTERRUPT - PC = IRQHANDLER
    }
}


void update_irqs_state(int *irqState) {
    irqState[0] = IORegister[0] & IORegister[3];
    irqState[1] = IORegister[1] & IORegister[4];
    irqState[2] = IORegister[2] & IORegister[5];
}

void read_from_disk(int* disk_sector, int* mem_buffer, int buffer){ //read from disk to mem
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        mem_buffer[i] = disk_sector[i];
        if (disk_sector[i]!=0) {
            dataMaxIndex = get_max(dataMaxIndex, buffer+i);
        }
    }
}

void write_to_disk(int* disk_sector, int* mem_buffer, int sector){
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        disk_sector[i] = mem_buffer[i];
        if (mem_buffer[i]!=0) {
            diskMaxIndex = get_max(diskMaxIndex, sector + i);
        }
    }
}


void diskIO_handler() {
    int type_of_operation, sector, buffer;
    type_of_operation = IORegister[14];
    if((type_of_operation != 0) & IORegister[17] == 0) { //Disk has a command that needs to be done and he is free.
        IORegister[17] = 1;
        diskCycleTimer = 0; //INITATE TIMER
        sector = IORegister[15];
        buffer = IORegister[16];
        //If fails, can't write/read disk sector to/from this address - program crashes.
        assert(buffer<MAX_DATA-SECTOR_SIZE);
        if(type_of_operation == 1) {//read operation...
            read_from_disk(&diskIO[sector], &MEM[buffer], buffer);
        }
        else{//write operation...
            write_to_disk(&diskIO[sector], &MEM[buffer], sector);
        }
    }
    else if(IORegister[17] == 1){//disk is not free.
        if(diskCycleTimer == 1023) { // FREE IO DISK
            IORegister[4] = 1; //IRQ STATUS
            IORegister[14] = 0; //DISK CMD
            IORegister[17] = 0; //DISK STATUS
        }
        diskCycleTimer += 1;
    }
}


void timer_handler() {
    if(IORegister[11] == 1) { //check if timer is enabled
        IORegister[12] += 1; //TIMER CURRENT += 1
    }
    if(IORegister[12] > IORegister[13]) {
        IORegister[12] = 0; // TIMER CURRENT = 0
        IORegister[3] = 1; //turn on irq0 bit for timer there is interrupt to handle. 
    }
    ////////END CHECK FOR TIMER INTERRUPT
}


int run_command(Instruction instruction) {
    R[1] = instruction.immediate1;
    R[2] = instruction.immediate2;
    FILE *fp = fopen(filenames[7],"a");
    write_to_file(fp, TRACE_LEN, trace);
    fclose(fp);
    if(instruction.op <= 8) {
        run_arithmetic(instruction,instruction.op);
    }
    else if(instruction.op <= 15) {
        run_jump_branch_commands(instruction , instruction.op);
    }
    else if(instruction.op <= 17) {
        run_memory_command(instruction,instruction.op);
    }
    else if(instruction.op <= 20) {
        run_IOregister_operation(instruction, instruction.op);
    }
    else if(instruction.op == 21) {
        cycles++;
        fp = fopen(filenames[5],"w");
        write_to_file(fp, DATA_LEN, data);
        fclose(fp);
        fp = fopen(filenames[12],"w");
        write_to_file(fp, DATA_LEN, disk);
        fclose(fp);
        fp = fopen(filenames[6],"w");
        write_to_file(fp, REG_HEX_LEN, registers);
        fclose(fp);
        fp = fopen(filenames[9],"w");
        write_to_file(fp, REG_HEX_LEN, cycle);
        fclose(fp);
        fp = fopen(filenames[13],"w");
        write_to_file(fp, 2, monitor);
        fclose(fp);
        
        //make_monitor_txt_file(); // TASK - MAKE FILE FOR MONITOR OUTPUT - EACH ROW IS VALUE IN HEXA, FIRST ROW 0,0 THEN 0,1 .... 1,0 , 1,1 - INDEXES - OUR ARRAY IS FLATTENED.
        exit(1); //determine exit message
    }
    else{
        fprintf(stderr, "op code not recognized"); //determine message
    }
}


void run_arithmetic(Instruction instruction, int id) {
    if(id == 0) {
        R[instruction.rd] = R[instruction.rs] + R[instruction.rt] + R[instruction.rm];
    }
    else if(id == 1) {
        R[instruction.rd] = R[instruction.rs] - R[instruction.rt] - R[instruction.rm];
    }
    else if(id == 2) {
        R[instruction.rd] = R[instruction.rs] * R[instruction.rt] + R[instruction.rm];
    }
    else if(id == 3) {
        R[instruction.rd] = R[instruction.rs] & R[instruction.rt] & R[instruction.rm];
    }
    else if(id == 4) {
        R[instruction.rd] = R[instruction.rs] | R[instruction.rt] | R[instruction.rm];
    }
    else if(id == 5) {
        R[instruction.rd] = R[instruction.rs] ^ R[instruction.rt] ^ R[instruction.rm];
    }
    else if(id == 6) {
        R[instruction.rd] = R[instruction.rs] << R[instruction.rt];
    }
    else if(id == 7) {
        R[instruction.rd] = R[instruction.rs] >> R[instruction.rt]; //need to do right arithmetic shift 
    }
    else if(id == 8) {
        R[instruction.rd] =(int) ((unsigned int)R[instruction.rs] >> R[instruction.rt]);
    }
}


void run_jump_branch_commands(Instruction instruction, int id) {
    int mask =  1 << 12;
    mask = mask - 1;
    int program_counter_new_address = R[instruction.rm] & mask;
    if(id == 9) {
        if(R[instruction.rs] == R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 10) {
        if(R[instruction.rs] != R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 11) {
        if(R[instruction.rs] < R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 12) {
        if(R[instruction.rs] > R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 13) {
        if(R[instruction.rs] <= R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 14) {
        if(R[instruction.rs] >= R[instruction.rt]) {
            PC = program_counter_new_address;
        }
    }
    else if(id == 15) {
        R[instruction.rd] = PC + 1;
        PC = program_counter_new_address;
    }
}


void run_memory_command(Instruction instruction , int id) {
    if(id == 16) {
        R[instruction.rd] = MEM[R[instruction.rs] + R[instruction.rt]] + R[instruction.rm]; 
    }
    else if(id == 17) {
        MEM[R[instruction.rs] + R[instruction.rt]] = R[instruction.rm] + R[instruction.rd];
        dataMaxIndex = get_max(dataMaxIndex, R[instruction.rs] + R[instruction.rt]);
    }
}


void run_IOregister_operation(Instruction instruction , int id) {
    if(id == 18) {
        inInterrupt = 0;
        PC = IORegister[7];
        return;
    }
    FILE *fp;
    fp = fopen(filenames[8],"a");
    write_to_file(fp, TRACE_LEN, hwregtrace);
    fclose(fp);
    if(id == 19) { //READ FROM IO REGISTER
        R[instruction.rd] = IORegister[R[instruction.rs] + R[instruction.rt]];
    }
    else if(id == 20) { // WRITE TO IO REGISTER
        IORegister[R[instruction.rs]+R[instruction.rt]] = R[instruction.rm];
        if(R[instruction.rs]+R[instruction.rt] == 9){
            fp = fopen(filenames[10],"a");
            write_to_file(fp, TXT_LEN, led);
            fclose(fp);
        }
        if(R[instruction.rs]+R[instruction.rt] == 10){
            fp = fopen(filenames[11],"a");
            write_to_file(fp, TXT_LEN, sevenseg);
            fclose(fp);
        }
    }

}



