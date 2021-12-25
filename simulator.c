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
    monitorFrame = malloc(MONITOR_RES * MONITOR_RES * sizeof(uint8_t));
    res = main_loop();
}

void update_monitor_pixels(){
    if(IORegister[22] == 1){
        uint8_t frame_data = IORegister[21];
        int frame_address = IORegister[20];
        monitorFrame[frame_address] = frame_data;
    }
}


int main_loop() {
    int oldPC, cycles = 0;
	while(cmdLst[PC]) {
        //Handle disk and timer
        diskIO_handler();
        timer_handler();
        update_irq2(cycles);
        update_monitor_pixels();

        //Handle interrupts
        interrupt_handler();

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

void read_from_disk(int* disk_sector, int* mem_buffer){ //read from disk to mem
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        mem_buffer[i] = disk_sector[i];
    }
}

void write_from_disk(int* disk_sector, int* mem_buffer){
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        disk_sector[i] = mem_buffer[i];
    }
}


void diskIO_handler() {
    int type_of_operation, sector, buffer;
    type_of_operation = IORegister[14];
    if((IORegister[14] != 0) & IORegister[17] == 0) { //Disk has a command that needs to be done and he is free.
        IORegister[17] = 1;
        diskCycleTimer = 0; //INITATE TIMER
        sector = IORegister[15];
        buffer = IORegister[16];
        if(type_of_operation == 1) {//read operation...
            read_from_disk(&diskIO[sector], &MEM[buffer]);
        }
        else{
            write_to_disk(&diskIO[sector], &MEM[buffer]);
        }
    }
    else if(IORegister[17] == 0){//disk is not free.
        if(diskCycleTimer == 1023) { // FREE IO DISK
            IORegister[4] = 1; //IRQ STATUS
            IORegister[14] = 0; //DISK CMD
            IORegister[17] = 0; //DISK STATUS
        }
        if(IORegister[17] == 1) { //TIMER IS STILL BUSY LET'S INCREMENT TIMER
            diskCycleTimer += 1;
        }
    }
}


void timer_handler() {
    if(IORegister[12] > IORegister[13]) {
        IORegister[12] = 0; // TIMER CURRENT = 0
        IORegister[3] = 1; //turn on irq0 bit for timer there is interrupt to handle. 
    }
    if(IORegister[11]) { //check if timer is enabled
        IORegister[12] += 1; //TIMER CURRENT += 1
    }
    ////////END CHECK FOR TIMER INTERRUPT
}


int run_command(Instruction instruction) {
    write_to_trace_txt(Instruction instruction);// TASK - WRITE TO TRACE.TXT LINE WITH PC , INST (12 DIGITS OF INSTRUCTION) AND REGISTERS BEFORE COMMAND - READ DESCRIPTION IN FILE BEFORE! 
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
        make_regout_txt_file(); // TASK - make file
        make_cycles_txt_file(); // TASK - MAKE FILE WITH NUM OF CYCLES.
        make_monitor_txt_file(); // TASK - MAKE FILE FOR MONITOR OUTPUT - EACH ROW IS VALUE IN HEXA, FIRST ROW 0,0 THEN 0,1 .... 1,0 , 1,1 - INDEXES - OUR ARRAY IS FLATTENED.
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
    int mask = 4095;
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
        MEM[R[instruction.rs] + R[instruction.rt]] = R[instruction.rm] + R[instruction.rt];
    }
}


void run_IOregister_operation(Instruction instruction , int id) {
    if(id == 18) {
        inInterrupt = 0;
        PC = IORegister[7];
    }
    else if(id == 19) { //READ FROM IO REGISTER
        R[instruction.rd] = IORegister[R[instruction.rs] + R[instruction.rt]];
        write_to_hregtrace_txt(instruction);// TASK - WRITE TO HREGTRACE.TXT FILE - CYCLE_NUM , READ OR WRITE OP - DEPENDS ON ID , NAME OF REGISTER FROM TABLE , DATA TO WRITE \ READ.
    }
    else if(id == 20) { // WRITE TI IO REGISTER
        if(R[instruction.rs]+R[instruction.rt] == 9){
            write_to_led_file();// TASK - WRITE TO LED FILE A LINE WITH CYCLE NUM , NEW LED VALUE.
        }
        if(R[instruction.rs]+R[instruction.rt] == 10){
            write_to_seven_seg_display_file(); // TASK - WRITE TO DISPLAY7SEG.TXT THE CYCLE NUM , NEW DISPLAY VALUE
        }

        IORegister[R[instruction.rs]+R[instruction.rt]] = R[instruction.rm];
        write_to_hregtrace_txt(instruction);// TASK - WRITE TO HREGTRACE.TXT FILE - CYCLE_NUM , READ OR WRITE OP - DEPENDS ON ID , NAME OF REGISTER FROM TABLE , DATA TO WRITE \ READ.
    }

}



