#include "simulator.h"

/*--------------------------------------------------Define Enum for reading / writing----------------------------------------------------*/
typedef enum {data, instruction, irq2, disk, registers, trace, asmfile, hwregtrace, leds, sevenseg, cycle, monitor} Mode;


/*------------------------------------------------------UTILS FUNC DECLARATIONS----------------------------------------------------------*/
int read_from_file(FILE *fp, int len, Mode mode);
void fill_with_null(int start, int end, Mode mode);
char cut_string_by_index(char *str, int i);
int compare (const void * a, const void * b);
void sign_ext(int *num);
void dec2hexa(char* result, int num, int len);
int hexa2dec(char *hex_rep, int len);
void dec2hexa(char* result, int num, int len);
int write_to_file(FILE *fp, int len, Mode mode);
void set_line_to_zero(char *line, int len);
int write_int_arr_to_file(FILE *fp, char *line, int line_len, int *arr, int arr_len);
int write_str_to_file(FILE *fp, char *line);
int get_max(int a, int b);


/*----------------------------------------------------SIMULATOR FUNC DECLARATIONS---------------------------------------------------------*/
/*-----------Read Functions-----------*/
int init_disk_lst(FILE *fp, char *line, int len);
int init_data_lst(FILE *fp, char *line, int len);
int init_inst_lst(FILE *fp, char *line, int len);
int init_irq2_lst(FILE *fp, char *line, int len);
int add_to_inst_lst(char *instruct, char *line);
int add_to_data_lst(int *mem, char *data);
int add_to_irq2_lst(int *irq2, char *data);
void read_from_disk(int* disk_sector, int* mem_buffer, int buffer);
/*----------Write Functions-----------*/
int write_diskout(FILE *fp, char *line, int len);
int write_dmemout(FILE *fp, char *line, int len);
int write_registers(FILE *fp, char *line, int len);
int write_trace(FILE *fp, char *line, int len);
int write_hwregtrace(FILE *fp, char *line, int len);
int write_led_7seg(FILE *fp, char *line, int len, int IORegIndex);
int write_cycle(FILE *fp, char *line, int len);
int write_monitor(FILE *fp, char *line, int len);
void write_to_disk(int* disk_sector, int* mem_buffer, int sector);


/*-------------------------------------------------------FUNC IMPLEMENTATIONS------------------------------------------------------------*/
char cut_string_by_index(char *str, int i) {
	char tmp = str[i];
    str[i] = '\0';
    return tmp;
}


int compare (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}


void sign_ext(int *num) {
    if (*num & 1<<11) {
        *num = *num | immMask;
    }
}


void dec2hexa(char* result, int num, int len){
    unsigned int uNum = (unsigned int) num;
    int currIdx;
    if (uNum != 0) {
        int i =0;
        while (uNum != 0){
            int tmp = uNum%16;
            currIdx = len-1-i;
            if (tmp <10) {result[currIdx] = (char) tmp+48;}
            else {result[currIdx] = (char) tmp+55;}
            i++;
            uNum = uNum/16;
        }
    }
}


int hexa2dec(char *hex_rep, int len){
    int ans = 0, ch_val;
    char ch;
    for (int idx = 0; idx < len; idx++){
         ans *= 16;
         ch = hex_rep[idx];
         if (ch>47 && ch< 58) ch_val = ((int) ch) - 48;
         else if (ch>64 && ch< 71) ch_val = ((int) ch) - 65 + 10;
         else ch_val = ((int) ch) - 97 + 10;  

        ans += ch_val;
    }
    return ans;
}


int get_max(int a, int b) {
    if (a>=b) {
        return a;
    }
    return b;
}


void set_line_to_zero(char *line, int len) {
    for(int i=0; i<len; i++) {
        line[i] = '0';
    }
}


/*------------------------------------------------------READING FROM FILE FLOW-----------------------------------------------------------*/
int read_from_file(FILE *fp, int len, Mode mode) {
    char *line = malloc(len*sizeof(char));
    int res = 0;
    switch (mode) {
        case (data): //Read dmemin
            res = init_data_lst(fp, line, len);
            break;
        case (instruction): //Read imemin
            res = init_inst_lst(fp, line, len);
            break;
        case (irq2): //Read irq2in
            res = init_irq2_lst(fp, line, len);
            break;
        case (disk):
            res = init_disk_lst(fp, line, len);
            break;
    }
    fill_with_null(res, MAX_INSTRUCTIONS, mode);
    free(line);
    return EXIT_SUCCESS;
}


int init_disk_lst(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp)) {
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_data_lst(&diskIO[i], line);
        i++;
    }
    return i;
}


int init_data_lst(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp)) {
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_data_lst(&MEM[i++], line);
    }
    return i;
}


int init_inst_lst(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp)) {
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_inst_lst(instructions[i++], line);
    }
    return i;
}


int init_irq2_lst(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp)) {
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_irq2_lst(&irq2Lst[i++], line);
    }
    return i;
}


int add_to_inst_lst(char *instruct, char *line) {
    strcpy(instruct, line);
    return 1;
}


int add_to_data_lst(int *mem, char *data) {
    *mem = (int)strtol(data, NULL, 16);
    return 1;
}


int add_to_irq2_lst(int *irq2, char *data) {
	*irq2 = (int)strtol(data, NULL, 10);
    return 1;
}


void fill_with_null(int start, int end, Mode mode) {
    int i;
    if (mode == data) { //Fill Data Arr
        for(i=start; i<end; i++) {
			MEM[i] = 0;
        }
    }
    if(mode == disk) {
        for(i=start; i < SECTOR_SIZE * NUM_SECTORS; i++) {
			diskIO[i] = 0;
        }
    }
    if (mode == instruction) { //Fill Instruction Arr
        for(i=start; i<end; i++) {
            instructions[i] = NULL;
        }
    }
    if (mode == irq2) { //Fill Irq2 Arr
        realloc(irq2Lst, (start+1)*sizeof(int));
        qsort(irq2Lst, start++, sizeof(int), compare);
        irq2Lst[start] = -1;
    }
}


/*-------------------------------------------------------WRITING TO FILE FLOW------------------------------------------------------------*/
int write_to_file(FILE *fp, int len, Mode mode) {
    char *line = malloc((len+1)*sizeof(char));
    line[len] = '\0';
    int res;
    switch (mode) {
        case (data): //Write dmemout
            res = write_dmemout(fp, line, len);
            break;
        case (disk):
            res = write_diskout(fp, line, len);
            break;
        case (registers): //Write imemout
            res = write_registers(fp, line, len);
            break;
        case (trace):
            res = write_trace(fp, line, len);
            break;
        case (hwregtrace):
            res = write_hwregtrace(fp, line, len);
            break;
        case (leds):
            res = write_led_7seg(fp, line, len, 9);
            break;
        case (sevenseg):
            res = write_led_7seg(fp, line, len, 10);
            break;
        case (cycle):
            res = write_cycle(fp, line, len);
            break;
        case (monitor):
            res = write_monitor(fp, line, len);
            break;
    }
    free(line);
    return res;
}


int write_diskout(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, diskIO, SECTOR_SIZE * NUM_SECTORS);
}


int write_dmemout(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, MEM, MAX_DATA-1);
}


int write_trace(FILE *fp, char *line, int len) {
    int j;
    char *regInHexa = malloc((REG_HEX_LEN+1)*sizeof(char));
    set_line_to_zero(regInHexa, REG_HEX_LEN);
    regInHexa[REG_HEX_LEN] = '\0';

    set_line_to_zero(line, TRACE_LEN);
    dec2hexa(line, PC, 3);
    line[3] = '\0';
    strcat(line, " ");
    strcat(line, instructions[PC]);
    strcat(line, " ");
    for(j=0; j<REGISTERS_LEN; j++) {
        dec2hexa(regInHexa, R[j], REG_HEX_LEN);
        strcat(line, regInHexa);
        strcat(line, " ");
        set_line_to_zero(regInHexa, REG_HEX_LEN);
    }
    line[TRACE_LEN-1] = '\0';
    write_str_to_file(fp, line);
    free(regInHexa);
    return EXIT_SUCCESS;
}


int write_hwregtrace(FILE *fp, char *line, int len) {
    Instruction inst = *cmdLst[PC];
    char *regInHexa = malloc((REG_HEX_LEN+1)*sizeof(char));
    set_line_to_zero(regInHexa, REG_HEX_LEN);
    regInHexa[REG_HEX_LEN] = '\0';
    itoa(cycles, line, 10);    
    if (inst.op == 19) {
        strcat(line, " READ ");
        dec2hexa(regInHexa, R[inst.rd], REG_HEX_LEN);
    }
    else if (inst.op == 20) {
        strcat(line, " WRITE ");
        dec2hexa(regInHexa, R[inst.rm], REG_HEX_LEN);
    }
    strcat(line, IORegNames[R[inst.rs]+R[inst.rt]]);
    strcat(line, " ");
    strcat(line, regInHexa);
    write_str_to_file(fp, line);
    free(regInHexa);
    return EXIT_SUCCESS;
}


int write_led_7seg(FILE *fp, char *line, int len, int IORegIndex) {
    char *regInHexa = malloc((REG_HEX_LEN+1)*sizeof(char));
    set_line_to_zero(regInHexa, REG_HEX_LEN);
    regInHexa[REG_HEX_LEN] = '\0';
    itoa(cycles, line, 10);    
    strcat(line, " ");
    dec2hexa(regInHexa, IORegister[IORegIndex], REG_HEX_LEN);
    strcat(line, regInHexa);
    write_str_to_file(fp, line);
    free(regInHexa);
    return EXIT_SUCCESS;
}


int write_cycle(FILE *fp, char *line, int len) {
    itoa(cycles, line, 10);  
    write_str_to_file(fp, line);
    return EXIT_SUCCESS;
}


int write_registers(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, R+3, REGISTERS_LEN-3);
}


int write_monitor(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, monitorFrame, MONITOR_RES * MONITOR_RES);
}


int write_str_to_file(FILE *fp, char *line) {
    fputs(line, fp);
    fputs("\n", fp);
    return EXIT_SUCCESS;
}


int write_int_arr_to_file(FILE *fp, char *line, int line_len, int *arr, int arr_len) {
    int i;
    for (i=0; i<arr_len; i++) {
        set_line_to_zero(line, line_len);
        dec2hexa(line, arr[i], line_len);
        write_str_to_file(fp, line);
    }
    return EXIT_SUCCESS;
}


void read_from_disk(int* disk_sector, int* mem_buffer, int buffer){ //read from disk to mem
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        mem_buffer[i] = disk_sector[i];
    }
}


void write_to_disk(int* disk_sector, int* mem_buffer, int sector){
    int i;
    for(i = 0 ; i < SECTOR_SIZE ; i++){
        disk_sector[i] = mem_buffer[i];
    }
}

