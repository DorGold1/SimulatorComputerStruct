#include "utils.c"

/*Input of Assembler.c is {program.asm imemin.txt dmemin.txt}
program.asm is the program to parse
imemin.txt is to instruction memory file
dmemin.txt is the data memory file
*/

/*---------------------------------------NOTES---------------------------------------

NEED TO FIX NULL HANDLING IN STRING MALLOC AND STRCPY
 HANDLE MEMORY LEAKS
*/

/*---------------------------------------Functions Implementations------------------------------------*/

//FUNCTIONS FROM C HEADER

int isLabel(char *str){
    int first_ch = (int) str[0];
    if ((first_ch > 64 && first_ch < 71) || (first_ch >96 && first_ch <103)) {return 1;}
    return 0;
}

void str2param (char* result, const char *str){
     for (int i = 0;  i < conversionSize;  i++){
        if (strcmp(str, conversionparam[i].str)==0) { strcpy(result,conversionparam[i].param_code); break;}
    }
}

void str2Hex (char *result, char *str){
    //gets hexa num or signed dec num as str--> returns into result its hexa rep
    int isHex, hex_len = strlen(result);
    isHex = isHexa(str);
    if (isHex) {
        int startIdx = hex_len-isHex;
        strcpy(result+startIdx, str+2);
    }
    else {
        int two_com_rep = str_to_2complement(str, hex_len);
        dec2hexa(result, two_com_rep, hex_len);
    }
}

int add_imm_to_result(char* result, char* str){
    if (isLabel(str)) {
        strcat(result, Label_flag);
        return 1;
    }
    else {
        char *tmp = (char *) malloc(3*sizeof(char));
        for (int i=0; i< 3; i++){ tmp[i] = '0';}
        str2Hex(tmp, str);
        strcat(result, tmp);
        return 0;
    }
}

void add_op_to_result(char* result, char* str){
    char *tmp = (char *) calloc(2,sizeof(char));
    str2param(tmp, str);
    strcpy(result, tmp);
}

void add_reg_to_result(char* result, char* str){
    char *tmp = (char *) calloc(1,sizeof(char));
    str2param(tmp, str);
    strcat(result, tmp);
}

int str_to_2complement(char* str, int hex_len){
    //gets decimal num as str and returns it as int in 2's complement rep
    int num = strtol(str, (char**) NULL, 10);
    int max_bits_idx = (hex_len*4)-1;
    if (num>=0) { return num;}
    else {
        int max_neg = 1<<max_bits_idx;
        int delta = max_neg+num;
        return (max_neg | delta);
    }
}

int isHexa (char* str){// check 0xnull case ??
   if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {return strlen(str+2);}
   return 0;
}

//FUNCTIONS FROM C FILE

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
            parseLabel(line[0], *pc);
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
    int hasLabel = 0;
    for (int i=0; i< 7; i++) {
        if (i==0) { add_op_to_result(result, line[0]);}
        else if (i<5) { add_reg_to_result(result, line[i]);}
        else {
            hasLabel = add_imm_to_result(result, line[i]);
            data_node *instruction_node = (data_node *) malloc(sizeof(data_node));
            make_new_node(instruction_node, line[i], pc);
            add_to_Queue(instruction_node, 0);
        }
    }
    // printf("the proccessed command is: %s\n", result);
    add_line_to_table(result, pc, 1);
}

void parseLabel(char *label_name, int pc){
    if (label_exist(label_name) == 0){
        data_node *new = (data_node *) malloc(sizeof(data_node));
        make_new_node(new, label_name, pc);
        add_to_Queue(new, 1);
    }
}

void make_new_node(data_node *new, char *name, int pc){
    int name_len = strlen(name);
    new->name = (char *) malloc(name_len*sizeof(char));
    strcpy(new->name, name);
    new->pc_num  = pc;
    new->next = NULL;
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

void add_to_Queue(data_node *new, int isLabel){
    Queue *currQ;
    currQ = isLabel == 1 ? Labels : instructions_with_label; 
    if (currQ->head == NULL){
        currQ->head = new;
        currQ->tail = new;
    }
    else {
        currQ->tail->next = new;
        currQ->tail = new;
    }
}

void add_line_to_table(char *parsedLine, int address, int is_intruction){
    //assuming tables have been initiliazed
    char **curr_table;
    curr_table = is_intruction == 1 ? imem_table : dmem_table;
    strcpy(curr_table[address], parsedLine);
}

int get_label_pc(char* search_term){
    data_node *curr;
    curr = Labels->head;
    while (curr != NULL) {
        if (strcmp(curr->name, search_term)== 0) return curr->pc_num;
        else curr = curr->next;
    }
    return -1;
}

void update_pc(int *pc, lineType lt){
    if (lt == InstructionOnly || lt == Label_Instruction) *pc = *pc +1;
}

void init_DS(){
    unparsed_instructions = (char **) malloc(MAX_ASSEMBLY_INSTRUCTIONS*sizeof(char *));
    for (int i =0; i< MAX_ASSEMBLY_INSTRUCTIONS; i++) {unparsed_instructions[i] = malloc(MAX_STR_LEN*sizeof(char));}
    imem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {imem_table[i] = malloc((iEntryLen+1)*sizeof(char));}
    dmem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {dmem_table[i] = malloc((dEntryLen+1)*sizeof(char));}
    Labels = (Queue *) malloc(sizeof(Queue));
    Labels->head = NULL;
    Labels->tail = NULL;
    instructions_with_label = (Queue *) malloc(sizeof(Queue));
    instructions_with_label->head = NULL;
    instructions_with_label->tail = NULL;
}

int label_exist(char *label_name){
    data_node *curr = Labels->head;
    while (curr != NULL){
        if (strcmp(curr->name, label_name) == 0) return 1;
        else curr = curr->next;
    }
    return 0;
}

void update_labled_instructions(){
    data_node *curr_instruction_node = instructions_with_label->head;
    char *parsed_instruction, *label_to_find, *imm_to_update, *hexa_pc, *empty_hexa_pc;
    hexa_pc = (char *) malloc(3*sizeof(char));
    empty_hexa_pc = (char *) malloc(3*sizeof(char));
    for (int i = 0; i < 3; i++) {hexa_pc[i] = '0';}

    while (curr_instruction_node != NULL){
        strcpy(hexa_pc, empty_hexa_pc);                                //empty the pc's hexa rep buffer
        parsed_instruction = imem_table[curr_instruction_node->pc_num];//get the instruction in hexa from table
        label_to_find = curr_instruction_node->name;
        imm_to_update = strstr(parsed_instruction, Label_flag);        //get the position of flag to be replaced
        //is it possible to have two labels in one instructions?
        dec2hexa(hexa_pc, get_label_pc(label_to_find), 3);             //get the hexa rep of the pc that will replace the Label_flag
        for (int i = 0; i < 3; i++) {imm_to_update[i] = hexa_pc[i];}
        curr_instruction_node = curr_instruction_node->next;
    }
}

/*--------------------------------------------------MAIN--------------------------------------------*/

int main(int argc, char** argv) {
    /*WORK FLOW*/
    FILE *fp;
    char **empty_buffer_line, **buffer_line;
    int line_len, pc = 0;
    //Init buffers & data structures
    buffer_line = (char **) malloc(MAX_LINE_LEN*sizeof(char *));
    empty_buffer_line = (char **) calloc(MAX_LINE_LEN,sizeof(char *));
    for (int i = 0; i <MAX_LINE_LEN; i++) {empty_buffer_line[i] = (char *) malloc(MAX_STR_LEN*sizeof(char));}
    init_DS();

    //Verify inputs
    if (argc != 4) {
        printf("Invalid amount of input arguments.\n");
        exit(EXIT_FAILURE);
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Failed to open %s.\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    //read from file into local array - sizes need further attention
    int read_res = read_from_file(fp, MAX_LINE_LEN, asmfile);   //read_res is the amount of lines read
    fclose(fp);

    //parse lines and add to local tables
    for (int i =0; i<read_res; i++){
        memcpy(buffer_line, empty_buffer_line, MAX_LINE_LEN*MAX_STR_LEN*sizeof(char));
        line_len = fix_line_for_parsing(buffer_line, unparsed_instructions[i]);
        parseLine(buffer_line, line_len, &pc);
    }

    //free buffers
    free(buffer_line);
    free(empty_buffer_line);
    buffer_line = NULL;
    empty_buffer_line = NULL;

    //second loop
    update_labled_instructions();   

    return EXIT_SUCCESS;
}