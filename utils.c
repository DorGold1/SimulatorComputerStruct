#include "simulator.h"
#include "Assembler.h"

//Mode enum for read/write file.
typedef enum {data, instruction, irq2, disk, registers, trace} Mode;


//Utils Func Declarations
int read_from_file(FILE *fp, int len, Mode mode);
void fill_with_null(int start, int end, Mode mode);
char cut_string_by_index(char *str, int i);
int compare (const void * a, const void * b);
void dec2hexa(char* result, int num, int len);
int hexa2dec(char *hex_rep, int len);
int write_to_file(FILE *fp, int len, Mode mode);
void set_line_to_zero(char *line, int len);
int write_int_arr_to_file(FILE *fp, char *line, int line_len, int *arr, int arr_len);


//Simulator Related Func
int init_data_lst(FILE *fp, char *line, int len);
int init_cmd_lst(FILE *fp, char *line, int len);
int init_irq2_lst(FILE *fp, char *line, int len);
int add_to_cmd_lst(Instruction *cmdLst, char *inst);
int add_to_data_lst(int *mem, char *data);
int add_to_irq2_lst(int *irq2, char *data);
int write_dmemout(FILE *fp, char *line, int len);
int write_registers(FILE *fp, char *line, int len);
int write_trace(FILE *fp, char *line, int len);


int read_from_file(FILE *fp, int len, Mode mode) {
    char *line = malloc(len*sizeof(char));
    int res;
    switch (mode) {
        case (data): //Read dmemin
            res = init_data_lst(fp, line, len);
            break;
        case (instruction): //Read imemin
            res = init_cmd_lst(fp, line, len);
            break;
        case (irq2): //Read irq2in
            res = init_irq2_lst(fp, line, len);
            break;
    }
    fill_with_null(res, MAX_INSTRUCTIONS, mode);
    free(line);
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


int init_cmd_lst(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp)) {
        if (strcmp(line,"\n") == 0) {
            continue;
        }
        add_to_cmd_lst(cmdLst[i++], line);
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
    return 1;
}


int add_to_irq2_lst(int *irq2, char *data) {
	*irq2 = (int)strtol(data, NULL, 10);
}


char cut_string_by_index(char *str, int i) {
	char tmp = str[i];
    str[i] = '\0';
    return tmp;
}


void fill_with_null(int start, int end, Mode mode) {
    int i;
    if (mode == 0) { //Fill Data Arr
        for(i=start; i<end; i++) {
			MEM[i] = 0;
        }
    }
    if (mode == 1) { //Fill Instruction Arr
        for(i=start; i<end; i++) {
            cmdLst[i] = NULL;
        }
    }
    if (mode == 2) { //Fill Irq2 Arr
        realloc(irq2Lst, (++start)*sizeof(int));
        qsort(irq2Lst, start, sizeof(int), compare);
        irq2Lst[start] = -1;
    }
}


int compare (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}


void dec2hexa(char* result, int num, int len){
    int currIdx;
    if (num != 0) {
        int i =0;
        while (num != 0){
            int tmp = num%16;
            currIdx = len-1-i;
            if (tmp <10) {result[currIdx] = tmp+48;}
            else {result[currIdx] = tmp+55;}
            i++;
            num = num/16;
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


int write_to_file(FILE *fp, int len, Mode mode) {
    char *line = malloc((len+1)*sizeof(char));
    line[len] = '\0';
    int res;
    switch (mode) {
        case (data): //Write dmemout
            res = write_dmemout(fp, line, len);
            break;
        case (registers): //Write imemout
            res = write_registers(fp, line, len);
            break;
        case (trace):
            res = write_trace(fp, line, len);
            break;
    }
}


int write_dmemout(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, MEM, MAX_DATA);
}


int write_trace(FILE *fp, char *line, int len) {
    set_line_to_zero(line, TRACE_LEN);
}


int write_str_to_file(FILE *fp, char *line) {
    fputs(line, fp);
    fputs("\n", fp);
}


int write_registers(FILE *fp, char *line, int len) {
    return write_int_arr_to_file(fp, line, len, R, REGISTERS_LEN);
}

int write_int_arr_to_file(FILE *fp, char *line, int line_len, int *arr, int arr_len) {
    int i;
    for (i=0; i<arr_len; i++) {
        set_line_to_zero(line, line_len);
        dec2hexa(line, arr[i], line_len);
        write_str_to_file(fp, line);
    }
    return 1;
}


void set_line_to_zero(char *line, int len) {
    for(int i=0; i<len; i++) {
        line[i] = '0';
    }
}