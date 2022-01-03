#include "assemblerutils.h"

/*---------------------------------------NOTES FOR SELF---------------------------------------

memory leaks ?
str_to_2complement - is integer overflow possible ?
main - sizes of buffers
main - should exit on errors ?
*/

/*---------------------------------------Functions Implementations------------------------------------*/

int isLabel(char *str){
    //returns 1 if str is label (labels starts with a letter)

    int first_ch = (int) str[0];
    if ((first_ch > 64 && first_ch < 91) || (first_ch >96 && first_ch <123)) {return 1;}
    return 0;
}

void num2hexa (char *result, char *str, int hex_len){
    //gets hexa num or signed dec num as str--> returns into result its hexa rep
    
    int str_is_hexa = isHexa(str);
    if (str_is_hexa) {
        int startIdx = hex_len-str_is_hexa;
        strcpy(result+startIdx, str+2);
    }
    else {
        int two_com_rep = str_to_2complement(str, hex_len); //what if str's value is too big for 2's complement rep ?
        dec2hexa(result, two_com_rep, hex_len);
    }
}

int add_imm_to_result(char* result, char* str){
    if (isLabel(str)) {
        strcat(result, Label_flag);
        return 1;
    }
    else {
        char tmp[4];
        for (int i=0; i< 3; i++){ tmp[i] = '0';}
        num2hexa(tmp, str, 3);
        strcat(result, tmp);
        return 0;
    }
}

void add_op_to_result(char* result, char* str){
    char tmp[3];
    str2param(tmp, str);
    strcpy(result, tmp);
}

void add_reg_to_result(char* result, char* str){
    char tmp[2];
    str2param(tmp, str);
    strcat(result, tmp);
}

void str2param (char* result, const char *str){
    //gets op/register string --> copying it hexa rep into result

     for (int i = 0;  i < conversionSize;  i++){
        if (strcmp(str, conversionparam[i].str)==0) {strcpy(result,conversionparam[i].param_code); break;}
    }
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

void init_DS(){
    unparsed_instructions = (char **) malloc(ASM_LINE_MAX*sizeof(char *));
    for (int i =0; i< ASM_LINE_MAX; i++) {unparsed_instructions[i] = malloc(ASM_LINE_BUFFER_LEN*sizeof(char));}

    imem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {imem_table[i] = malloc((iEntryLen+1)*sizeof(char));}

    char dmem_empty_entry[9] = {'0', '0', '0', '0', '0', '0', '0', '0', 0};
    dmem_table = (char **) malloc(memSize*sizeof(char *));
    for (int i = 0; i< memSize; i++) {dmem_table[i] = malloc((dEntryLen+1)*sizeof(char)); strcpy(dmem_table[i], dmem_empty_entry);}

    Labels = (Queue *) malloc(sizeof(Queue));
    Labels->head = NULL;
    Labels->tail = NULL;

    instructions_with_label = (Queue *) malloc(sizeof(Queue));
    instructions_with_label->head = NULL;
    instructions_with_label->tail = NULL;
}

int fix_line_for_parsing(char **broken_res, char *lineBuffer){
    //gets allocated memory pointer for result
    int line_len;
    line_len = break_buffer(broken_res, lineBuffer);
    drop_comment(broken_res, &line_len);
    return line_len;
}

int break_buffer(char **broken_buffer, char *lineBuffer){
    int bufferLen = 0; 
    char delim[4] = {',', ' ', '\t'};

    char *token = strtok(lineBuffer, delim);
    if (token == NULL) return bufferLen; //empty line case
    strcpy(broken_buffer[bufferLen++], token);
    if (strstr(token, "#") != NULL) return bufferLen; //comment-only  line case

    while((token = strtok(NULL, delim)) != NULL){
        strcpy(broken_buffer[bufferLen++], token);
        if (strstr(token, "#") != NULL) break; //found comment - can return
        if (bufferLen==MAX_LINE_LEN) break; // max legal length - can return
    }
    return bufferLen;
}

void drop_comment(char **line, int *line_len){
    char *curr_word, *found;
    int i;
    for (i = 0; i< *line_len; i++){
        curr_word = line[i];
        if ((found = strstr(curr_word, "#")) != NULL) {
            *found = '\0';
            *line_len = i;
            return;
        }
    }
    
}

void parseLine(char **line, int line_len, int *pc, int *dtable_last_idx){
    lineType lt = get_lineType(line_len);
    switch (lt)
    {
        case LabelOnly:
            parseLabel(line[0], *pc);
            break;
        case WordOnly:
            parseWord(line, dtable_last_idx);
            break;
        case Label_Instruction:
            //parse Label and then instruction
            parseLabel(line[0], *pc);
            parseInstruction(&line[1], *pc);
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

void parseLabel(char *label_name, int pc){
    if (find_node(label_name, 1) == NULL){
        data_node *new = (data_node *) malloc(sizeof(data_node));
        label_name[strcspn(label_name, ":")] = '\0';        //kick out the ":" from label
        make_new_node(new, label_name, pc);
        add_to_Queue(new, 1);
    }
}

void parseWord (char **line, int *dtable_last_idx){
    char *address, *data, hex_address[13], hex_data[dEntryLen+1];
    int dec_address, dec_data, data_hex_len, max_negative;
    address = line[1];
    data = line[2];

    for (int i = 0; i <dEntryLen; i++) {hex_data[i] = '0';}
    for (int i = 0; i <12; i++) {hex_address[i] = '0';}
    
    //getting decimal value of address
    num2hexa(hex_address, address, 12);
    dec_address = hexa2dec(hex_address, 12);

    //getting hexa value of data
    if((data_hex_len = isHexa(data)) != 0){
        //data is hexa
        dec_data = hexa2dec(&data[2], data_hex_len);
        max_negative = 1<<(data_hex_len*4-1);
            //in case data need sign extention (last bit in binary rep is set)
        if (dec_data & max_negative) {for (int i = 0; i <dEntryLen-data_hex_len; i++) {hex_data[i] = 'F';}}
    }
    //data is decimal
    else {dec_data = (int) strtol(data, NULL, 10);}
    dec2hexa(hex_data, dec_data, dEntryLen);

    add_line_to_table(hex_data, dec_address, 0);
    update_dtable_last_idx(dtable_last_idx, dec_address);
}

void parseInstruction (char **line, int pc){

    char result[iEntryLen+1];
    int imm_is_label = 0;
    
    //getting the parsed instruction into result
    for (int i=0; i< 7; i++) {
        if (i==0) { add_op_to_result(result, line[0]);}
        else if (i<5) { add_reg_to_result(result, line[i]);}
        else {
            imm_is_label = add_imm_to_result(result, line[i]);
            if (imm_is_label){
                data_node *instruction_node = (data_node *) malloc(sizeof(data_node));
                make_new_node(instruction_node, line[i], pc);
                add_to_Queue(instruction_node, 0);
            }
        }
    }
    add_line_to_table(result, pc, 1);
}

void add_line_to_table(char *parsedLine, int address, int is_intruction){
    //assuming tables have been initiliazed
    char **curr_table;
    curr_table = is_intruction == 1 ? imem_table : dmem_table;
    strcpy(curr_table[address], parsedLine);
}

void update_pc(int *pc, lineType lt){
    if (lt == InstructionOnly || lt == Label_Instruction) *pc = *pc +1;
}

void make_new_node(data_node *new, char *name, int pc){
    int name_len = strlen(name)+1;
    new->name = (char *) malloc(name_len*sizeof(char));
    strcpy(new->name, name);
    new->pc_num  = pc;
    new->next = NULL;
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

data_node* find_node(char* search_term, int LabelQ){
    data_node *curr;
    curr = LabelQ == 1 ? Labels->head : instructions_with_label->head;
    while (curr != NULL){
        if (strcmp(search_term, curr->name)==0) return curr;
        else {curr = curr->next;}
    }
    return NULL;
}

void update_labled_instructions(){
    data_node *curr_instruction_node = instructions_with_label->head;
    char *parsed_instruction, *label_to_find;
    int dec_pc, flag_idx;
    
    //init buffer & empty buffer
    char hexa_pc[4];
    char empty_hexa_pc[4] = {'0', '0', '0', 0};

    while (curr_instruction_node != NULL){

        //get data from Queue
        parsed_instruction = imem_table[curr_instruction_node->pc_num];
        label_to_find = curr_instruction_node->name;
        flag_idx = strstr(parsed_instruction, Label_flag) - parsed_instruction;
        dec_pc = find_node(label_to_find, 1)->pc_num;
        
        //update hexa_pc into instruction
        strcpy(hexa_pc, empty_hexa_pc);
        dec2hexa(hexa_pc, dec_pc, 3);
        for (int i = 0; i < 3; i++) {parsed_instruction[flag_idx+i] = hexa_pc[i];}

        curr_instruction_node = curr_instruction_node->next;
    }
}

void copy_table_to_file(char **table, int table_size, char *file_path){
    FILE *fp;
    fp = fopen(file_path, "w");
    if (fp == NULL) {printf("Failed to open: %s\n", file_path); exit(EXIT_FAILURE);}
    for (int i = 0; i < table_size; i++){
        fprintf(fp, "%s\n", table[i]);
    }
    fclose(fp);
}

void update_dtable_last_idx (int *dtable_last_idx, int curr_address){
    if (*dtable_last_idx < curr_address) *dtable_last_idx = curr_address;
}

void free_table(char **table, int start, int end){
    for (int i = start; i< end; i++) {
        free(table[i]);
        table[i] = NULL;
    }
}

void free_Queues(){
    data_node *curr, *tmp;

    curr = Labels->head;
    while(curr != NULL){
        tmp = curr->next;
        free(curr);
        curr = tmp;
    }

    curr = instructions_with_label->head;
    while(curr != NULL){
        tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    if (curr != NULL) {free(curr);}
    if (tmp != NULL) {free(tmp);}
}
/*--------------------------------------------------MAIN--------------------------------------------*/

int main(int argc, char** argv) {
    FILE *fp;
    char **buffer_line, *program_path, *imem_file, *dmem_file;
    int line_len, lines_read_from_asm, pc = 0, dtable_last_idx = 0;
    
    //Init buffers & data structures
    buffer_line = (char **) malloc(MAX_LINE_LEN * sizeof(char *));
    for (int i = 0; i <MAX_LINE_LEN; i++) {buffer_line[i] = (char *) calloc(MAX_STR_LEN, sizeof(char));}
    init_DS();

    //Verify inputs
    if (argc != 4) {printf("Invalid amount of input arguments.\n"); exit(EXIT_FAILURE);}
    program_path = argv[1];
    imem_file = argv[2];
    dmem_file = argv[3];

    fp = fopen(program_path, "r");
    if (fp == NULL) {printf("Failed to open asm file : %s\n", program_path); exit(EXIT_FAILURE);}
    
    //read from file into unparsed_instructions local table - sizes need further attention
    lines_read_from_asm = init_unparsed_instructions(fp, ASM_LINE_BUFFER_LEN);
    fclose(fp);
    
    //parse lines and add them to local tables
    for (int i =0; i<lines_read_from_asm; i++){
        line_len = fix_line_for_parsing(buffer_line, unparsed_instructions[i]);
        parseLine(buffer_line, line_len, &pc, &dtable_last_idx);
    }

    //free buffers & local copy of asm file
    
    //second loop
    update_labled_instructions();
    free_Queues();
    free(Labels); Labels = NULL;
    free(instructions_with_label); instructions_with_label = NULL;
                                            /*here: pc is larger by 1 from the last iEntry_idx because update_pc() was called by halt (function that must be called in the end of asm file)
                                              dtable_last_idx is exactly true to its name
                                              */

    //copy from imem_table & dmem_table to corresponding files (paths from input)
    copy_table_to_file(imem_table, pc, imem_file);
    copy_table_to_file(dmem_table, dtable_last_idx+1, dmem_file);

    //free tables completely
    free_table(imem_table, 0, memSize); imem_table = NULL;
    free_table(dmem_table, 0, memSize); dmem_table = NULL;
    free_table(unparsed_instructions, 0, ASM_LINE_MAX); unparsed_instructions = NULL;
    return EXIT_SUCCESS;
}