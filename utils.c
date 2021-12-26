#include "simulator.h"
#include "Assembler.h"

//Mode enum for read/write file.
typedef enum {data, instruction, irq2, disk, registers, trace, asmfile} Mode;


//Utils Func Declarations
int read_from_file(FILE *fp, int len, Mode mode);
void fill_with_null(int start, int end, Mode mode);
char cut_string_by_index(char *str, int i);
int compare (const void * a, const void * b);
void sign_ext(int *num);
void dec2hexa(char* result, int num, int len);
int hexa2dec(char *hex_rep, int len);
int write_to_file(FILE *fp, int len, Mode mode);
void set_line_to_zero(char *line, int len);
int write_int_arr_to_file(FILE *fp, char *line, int line_len, int *arr, int arr_len);
int write_str_to_file(FILE *fp, char *line);



//Simulator Related Func
int init_data_lst(FILE *fp, char *line, int len);
int init_cmd_lst(FILE *fp, char *line, int len);
int init_irq2_lst(FILE *fp, char *line, int len);
int add_to_inst_lst(char *instruct, char *line);
int add_to_data_lst(int *mem, char *data);
int add_to_irq2_lst(int *irq2, char *data);
int write_dmemout(FILE *fp, char *line, int len);
int write_registers(FILE *fp, char *line, int len);
int write_trace(FILE *fp, char *line, int len);


//Assembler Related Func
int init_unparsed_instructions(FILE *fp, char *line, int len);




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
        case (asmfile): //Read asm file
            res = init_unparsed_instructions(fp, line, len);
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
            instructions[i] = NULL;
        }
    }
    if (mode == 2) { //Fill Irq2 Arr
        realloc(irq2Lst, (++start)*sizeof(int));
        qsort(irq2Lst, start, sizeof(int), compare);
        irq2Lst[start] = -1;
    }
    if (mode == asmfile) {}
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
        dec2hexa(regInHexa, R[j], DATA_LEN-1);
        strcat(line, regInHexa);
        strcat(line, " ");
        set_line_to_zero(regInHexa, REG_HEX_LEN);
    }
    line[TRACE_LEN] = '\0';
    write_str_to_file(fp, line);
    int a=3;
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

int init_unparsed_instructions(FILE *fp, char *line, int len) {
    int i = 0;
    while(fgets(line, len, fp) != NULL) {
        strcpy(unparsed_instructions[i++], line);
    }
    return i;
}