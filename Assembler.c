#include "utils.c"

/*Input of Assembler.c is {program.asm imemin.txt dmemin.txt}
program.asm is the program to parse
imemin.txt is to instruction memory file
dmemin.txt is the data memory file
*/

/*--------------------------------------------GLOBAL STUFF---------------------------------------------*/
char **imem_table, **dmem_table;
label_list *Labels;

/*---------------------------------------Functions Declarations---------------------------------------*/

void init_DS();
void verify_input(int argc, char *program_path, FILE *fp);

/*gets line as read from file and getting it ready for parsing*/
int fix_line_for_parsing(char** broken_res, char *lineBuffer);
/*Helping Functions*/
int break_buffer(char **broken_buffer, char *lineBuffer);
void drop_comment(char **line, int line_len);
/*end of helping functions*/

/*gets a fixed line from asm file as line and adds it parsed to relevant table*/
void parseLine(char **line, int lLength, int *pc);
/*Helping Functions*/
void parseInstruction(char** line, int pc);
void parseLabel(char **line, int pc);
void parseWord (char **line);

void add_line_to_table(char *parsedLine, int hex_address, int is_intruction);
void update_pc(int *pc, lineType lt);
/*end of helping functions*/

/*returns the type of line currently being read*/
lineType get_lineType(int line_len);

/*Labels Queue - Helping Functions*/
void add_new_label(label_node *new);
int get_label_pc(char* search_term);
/*end of helping functions*/

/*---------------------------------------Functions Implementations------------------------------------*/

int fix_line_for_parsing(char **broken_res, char *lineBuffer){
    //gets allocated memory pointer for result
    int line_len;
    line_len = break_buffer(broken_res, lineBuffer);
    drop_comment(broken_res, line_len);
    return line_len;
}

int break_buffer(char **broken_buffer, char *lineBuffer){
    int bufferLen = 0; 
    char delim[4] = {',', ' ', '\t'};

    char *token = strtok(lineBuffer, delim);
    broken_buffer[bufferLen++] = token;
    while((token = strtok(NULL, delim)) != NULL){
        broken_buffer[bufferLen++] = token;
        if (bufferLen==MAX_LINE_LEN) break;
    }
    return bufferLen;
}

void drop_comment(char **line, int line_len){
    char *last_str = line[line_len-1];
    int N = strlen(last_str);
    
    for (int i=0; i<N; i++) {if(last_str[i] == '#') {last_str[i] = '\0'; break;}}
}

void parseLine(char **line, int line_len, int *pc){
    lineType lt = get_lineType(line_len);
    switch (lt)
    {
        case LabelOnly:
            parseLabel(line, *pc);
            break;
        case WordOnly:
            parseWord(line);
            break;
        case Label_Instruction:
            //parse Label and then instruction
            parseLabel(line[0], *pc);
            parseInstruction(line+1, *pc);
            break;
        case Empty:
            //parseEmpty is nothing
            break;
        default:
            //InstructionOnly
            parseInstruction(line, *pc);
            break;
    }
    update_pc(pc, lt);
}

lineType get_lineType(int line_len){
    //assuming fix_line_for_parsing went well, those cases are the only options
    switch(line_len){
        case 0:
            return Empty;
        case 1:
        //label will have : next to label's name
            return LabelOnly;
        case 3:
            return WordOnly;
        case 7:
            return InstructionOnly;
        default:
            return Label_Instruction;
    }
}

void parseInstruction (char **line, int pc){

    char *result = (char *) calloc(iEntryLen, sizeof(char));
    //getting the parsed instruction into result
    for (int i=0; i< 7; i++) {
        if (i==0) { add_op_to_result(result, line[0]);}
        else if (i<5) { add_reg_to_result(result, line[i]);}
        else { add_imm_to_result(result, line[i]);}//need to set case for label
    }
    printf("the proccessed command is: %s\n", result);
    add_line_to_table(result, pc, 1);
}

void parseLabel(char **line, int pc){
    label_node *new;
    int name_len;

    new = (label_node *) malloc(sizeof(label_node));
    name_len = strlen(line[0]);

    new->name = (char *) malloc(name_len*sizeof(char));
    strcpy(new->name, line[0]);
    new->pc_num  = pc;
    new->next = NULL;

    add_new_label(new);
}

void parseWord (char **line){
    char *address, *data, *hex_address, *hex_data;
    int dec_address;
    address = line[1];
    data = line[2];

    hex_data = (char *) malloc(dEntryLen*sizeof(char));
    hex_address = (char *) malloc(12*sizeof(char));
    str2Hex(hex_data, data);
    str2Hex(hex_address, address);
    dec_address = hexa2dec(hex_address, 12);
    add_line_to_table(hex_data, dec_address, 0);
}

void add_new_label(label_node *new){
    if (Labels->head == NULL){
        Labels->head = new;
        Labels->tail = new;
    }
    else {
        Labels->tail->next = new;
        Labels->tail = new;
    }
}

void add_line_to_table(char *parsedLine, int address, int is_intruction){
    //assuming tables have been initiliazed
    char **curr_table;
    curr_table = is_intruction == 1 ? imem_table : dmem_table;
    strcpy(curr_table[address], parsedLine);
}

int get_label_pc(char* search_term){
    label_node *curr;
    curr = Labels->head;
    while (curr != NULL) {
        if (strcmp(curr->name, search_term)== 0) return curr->pc_num;
        else curr = curr->next;
    }
    return -1;
}

void update_pc(int *pc, lineType lt){
    if (lt == InstructionOnly || lt == Label_Instruction) *pc++;
}

void init_DS(){
    imem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {imem_table[i] = malloc((iEntryLen+1)*sizeof(char));}
    dmem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {dmem_table[i] = malloc((dEntryLen+1)*sizeof(char));}
    Labels = (label_list *) malloc(sizeof(label_list));
    Labels->head = NULL;
    Labels->tail = NULL;
}

void verify_input(int argc, char *program_path, FILE *fp){
    if (argc != 4) {
        printf("Invalid amount of input arguments.\n");
        exit(EXIT_FAILURE);
    }
    fp = fopen(program_path, "r");
    if (fp == NULL) {
        printf("Failed to open %s.\n", program_path);
        exit(EXIT_FAILURE);
    }
}
/*--------------------------------------------------MAIN--------------------------------------------*/

int main(int argc, char** argv) {
    /*WORK FLOW*/
    FILE *fp;
    char *lineBuffer = NULL, **ready_line;
    int bytes_read, line_len, lineBuffer_size = 0, pc = 0;
    
    init_DS();
    verify_input(argc, argv[1], fp);
    ready_line = (char **) malloc(MAX_LINE_LEN*sizeof(char *));
    for (int i =0; i<MAX_LINE_LEN; i++) {ready_line[i] = (char *) calloc(MAX_STR_LEN,sizeof(char));}

    while ((bytes_read = getline(&lineBuffer, &lineBuffer_size, fp)) != -1) {
        
        line_len = fix_line_for_parsing(ready_line, lineBuffer);
        parseLine(ready_line, line_len, &pc);
            //is it legal?
        for (int i=0; i<MAX_LINE_LEN; i++) {free(ready_line[i]; ready_line[i]=NULL);}
        ready_line = NULL;  //suppose to re-init the line
        // free(ready_line);
    }
    //verify end of file || error ??
    if (lineBuffer != NULL) free(lineBuffer);
    lineBuffer = NULL;  
    fclose(fp);
    //second pass - updating labels
    //copying tables to files
    

    /*TESTING-PART MAIN*/
    // char **broken_line;
    // char test_line[63] = "	add $sp, $sp, $imm2, $zero, 0, -3		# adjust stack for 3 items";
    // printf("test line is %s\nits length is %d\n", test_line, strlen(test_line));
    // fix_line_for_parsing(broken_line, test_line);
    // printf("result of fixing is:\n");
    // for (int i =0; i< MAX_LINE_LEN; i++){
    //     printf("broken at index %d is:\t%s", i, broken_line[i]);
    // }
    
    return 0;
}