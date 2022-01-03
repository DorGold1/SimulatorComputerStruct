#include "simulatorutils.h"


/*----------------------------------------------------------------INIT FLOW----------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    int i;
    FILE *fp;
    filenames = argv;
    //Create txt files for appending
    init_txt_files();

    //Reading Instructions Flow
    fp = fopen(filenames[1],"r");
    instructions = malloc(MAX_INSTRUCTIONS * sizeof(char *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        instructions[i] = malloc(INSTRUCTION_LEN * sizeof(char));
    }
    read_from_file(fp, INSTRUCTION_LEN, instruction);
	cmdLst = malloc(MAX_INSTRUCTIONS * sizeof(Instruction *));
    for(i=0; i<MAX_INSTRUCTIONS; i++) {
        cmdLst[i] = (Instruction *)malloc(sizeof(Instruction *));
        add_to_cmd_lst(cmdLst[i], instructions[i]);
    }
	fclose(fp);

    //Reading dmemin Flow
    fp = fopen(filenames[2],"r");
	read_from_file(fp, REG_HEX_LEN+1, data);
    fclose(fp);

    //Reading disk Flow
    diskIO = calloc(NUM_SECTORS * SECTOR_SIZE ,sizeof(int));
    if ((fp = fopen(filenames[3], "r")) == NULL) {
        fp = fopen(filenames[3], "w");
    }
	read_from_file(fp, NUM_SECTORS * SECTOR_SIZE, disk);
    fclose(fp);

    //Reading irq2 Flow
    irq2Lst = calloc(MAX_INSTRUCTIONS, sizeof(int));
    if ((fp = fopen(filenames[4], "r")) == NULL) {
        fp = fopen(filenames[4], "w");
    }
    read_from_file(fp, INSTRUCTION_LEN, irq2);
    fclose(fp);

    //Initializing the rest of the IO devices
    monitorFrame = calloc(MONITOR_RES * MONITOR_RES ,sizeof(int));
    for (i=0; i<MONITOR_RES * MONITOR_RES; i++) {
        monitorFrame[i]=0;
    }
    
    //Run main_loop - start executing commands.
    main_loop();
}


void init_txt_files() {
    FILE *fp;
    fp = fopen(filenames[7],"w");
    fclose(fp);
    fp = fopen(filenames[8],"w");
    fclose(fp);
    fp = fopen(filenames[10],"w");
    fclose(fp);
    fp = fopen(filenames[11],"w");
    fclose(fp);
}


void add_to_cmd_lst(Instruction *cmdLst, char *inst) {
    //This function cuts instruction on given spots and generates struct Instruction with required fields (op, rs, rt, etc.)
    if (inst == NULL) {
        cmdLst = NULL;
        return;
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
}


/*---------------------------------------------------------------MAINLOOP FLOW-------------------------------------------------------------*/
void main_loop() {
    int res, oldPC;
	while(cmdLst[PC]){
        //Handle interrupts
        interrupt_handler();

        //Handle disk, timer, irq2, monitor
        diskIO_handler();
        timer_handler();
        update_irq2(cycles);
        update_monitor_pixels();

        //Execute current command
        oldPC = PC; 
        res = run_command(*cmdLst[PC]);

        //Update PC. If PC has changed with a branch jump command - it doesn't need to be increased by 1.
        if(PC == oldPC) { 
            PC += 1;
        }

        //Update cycle count
        cycles+=1;
        IORegister[8] = cycles;

        //Check If Finished / Crashed.
        if (res == EXIT_SUCCESS) {
            write_exit_txt_files();
            fprintf(stdout, "Simulator Exited Successfully.");
            exit(EXIT_SUCCESS);
        }
        if (res == EXIT_FAILURE) {
            fprintf(stderr, "ERROR - Simulator failed.\nPC - %d\nCycle - %d\n", PC, cycles);
            exit(EXIT_FAILURE);
        }
    }
    write_exit_txt_files();
    fprintf(stderr, "ERROR - Got to the last instruction - without HALT.\nWrote out txt files - based on current state.\nPC - %d\nCycle - %d\n", PC, cycles);
    exit(EXIT_FAILURE);
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


void diskIO_handler() {
    int type_of_operation, sector, buffer;
    type_of_operation = IORegister[14];
    if((type_of_operation != 0) & (IORegister[17] == 0)) { //Disk has a command that needs to be done and he is free.
        IORegister[17] = 1;
        diskCycleTimer = 0; //INITATE TIMER
        sector = IORegister[15];
        buffer = IORegister[16];
        //If fails, can't write/read disk sector to/from this address - program crashes.
        if(buffer>MAX_DATA-SECTOR_SIZE){
            fprintf(stderr, "ERROR - read/write disk - Not enough space on MEM\nReading/Writing indexes - %d to %d - Memory Overflow. (Max size - 4096)\nPC - %d\nCycle - %d\n", buffer, buffer + SECTOR_SIZE, PC, cycles);
            exit(EXIT_FAILURE);
        }
        if(type_of_operation == 1) {//read operation...
            read_from_disk(&diskIO[sector * SECTOR_SIZE], &MEM[buffer], buffer);
        }
        else{//write operation...
            write_to_disk(&diskIO[sector * SECTOR_SIZE], &MEM[buffer], sector);
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


void update_irq2(int cycle) {
    if (irq2Lst[irq2Index] == cycle) {
        IORegister[5] = 1;
        irq2Index++;
    }
}


void update_monitor_pixels(){
    if(IORegister[22] == 1){
        int frame_data = IORegister[21];
        int frame_address = IORegister[20];
        monitorFrame[frame_address] = frame_data;
        IORegister[22] = 0;
    }
}


void write_exit_txt_files() {
    FILE *fp;
    fp = fopen(filenames[5],"w");
    write_to_file(fp, REG_HEX_LEN, data);
    fclose(fp);
    fp = fopen(filenames[12],"w");
    write_to_file(fp, REG_HEX_LEN, disk);
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
    fp = fopen(filenames[14],"wb");
    write_to_file(fp, 2, monitoryuv);
    fclose(fp);
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
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
    return CONTINUE_MAINLOOP;
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
            write_to_file(fp, TXT_LEN, leds);
            fclose(fp);
        }
        if(R[instruction.rs]+R[instruction.rt] == 10){
            fp = fopen(filenames[11],"a");
            write_to_file(fp, TXT_LEN, sevenseg);
            fclose(fp);
        }
    }

}


