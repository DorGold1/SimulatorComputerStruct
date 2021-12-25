/*---------------------------------------------------------------Define Constants-------------------------------------------------------------*/
#define memSize 4096
#define MAX_STR_LEN 100
#define MAX_LINE_LEN 8
#define conversionSize 38
#define dEntryLen 8
#define iEntryLen 12

/*---------------------------------------------------------------FUNCTIONS & TYPEDEF DECLAREATIONS--------------------------------------------*/
int isletter(char c);
int isHexa (char* str); /*returns 0 if str isn't hex rep of number, otherwise returns its length*/
int str_to_2complement(char* str, int max_bits_idx);  /*get number as a string and last idx of binary rep - returns it's value in 2's complement rep*/
void dec2hexa(char* result, int num, int hex_len);
int hexa2dec(char *hex_rep, int len);
void str2param (char* result, const char *str);
void str2Hex (char* result, char *str);
void add_op_to_result(char* result, char* str);
void add_reg_to_result(char* result, char* str);
void add_imm_to_result(char* result, char* str);

typedef struct label_node label_node;
typedef struct label_list label_list;
/*---------------------------------------------------------------ENUMS & STRUCTS---------------------------------------------------------------*/
struct label_node{
    char* name;
    int pc_num;
    label_node* next;
};

struct label_list{
    label_node *head;
    label_node *tail;
};

typedef enum {
    LabelOnly,
    InstructionOnly,
    WordOnly,
    Label_Instruction,
    Empty
} lineType;

typedef enum {
    reg,
    op,
    imm
} paramType;

/*---------------------------------------------------------------CONVERSION TABLE---------------------------------------------------------------*/
const static struct {//used for op/reg
    const char *param_code;
    const char *str;
} conversionparam [] = {
    {"00", "add"},
    {"01", "sub"},
    {"02", "mac"},
    {"03", "and"},
    {"04", "or"},
    {"05", "xor"},
    {"06", "sll"},
    {"07", "sra"},
    {"08", "srl"},
    {"09", "beq"},
    {"0A", "bne"},
    {"0B", "blt"},
    {"0C", "bgt"},
    {"0D", "ble"},
    {"0E", "bge"},
    {"0F", "jal"},
    {"10", "lw"},
    {"11", "sw"},
    {"12", "reti"},
    {"13", "in"},
    {"14", "out"},
    {"15", "halt"},
    {"0", "$zero"},
    {"1", "$imm1"},
    {"2", "$imm2"},
    {"3", "$v0"},
    {"4", "$a0"},
    {"5", "$a1"},
    {"6", "$a2"},
    {"7", "$t0"},
    {"8", "$t1"},
    {"9", "$t2"},
    {"A", "$s0"},
    {"B", "$s1"},
    {"C", "$s2"},
    {"D", "$gp"},
    {"E", "$sp"},
    {"F", "$ra"},
};

/*---------------------------------------------------------------FUNCTIONS IMPLEMENTIONS-------------------------------------------------------*/
void str2param (char* result, const char *str){
     for (int i = 0;  i < conversionSize;  i++){
        if (strcmp(str, conversionparam[i].str)==0) { strcpy(result,conversionparam[i].param_code); break;}
    }
}

void str2Hex (char *result, char *str){         /*function is called only after labels have been saved*/
    int isHex, hex_len = strlen(result);
    //if (isletter(str[0])) {/*parse as label*/}
    if (0) {}
    else{ 
        isHex = isHexa(str);
        if (isHex) {strcpy(result+(hex_len-isHex), str+2);}
        else {dec2hexa(result, str_to_2complement(str, (hex_len*4)-1));}
    }
}

void add_imm_to_result(char* result, char* str){
    char *tmp = (char *) malloc(3*sizeof(char));
    for (int i=0; i< 3; i++){ tmp[i] = '0';}
    str2Hex(tmp, str);
    strcat(result, tmp);
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

void dec2hexa(char* result, int num){
    int currIdx, hex_len = strlen(result);
    if (num != 0) {
        int i =0;
        while (num != 0){
            int tmp = num%16;
            currIdx = hex_len-1-i;
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

int str_to_2complement(char* str, int max_bits_idx){
    int num = strtol(str, (char**) NULL, 10);
    if (num>=0) { return num;}
    else {
        int max_neg = 1<<max_bits_idx;//immediate last bits' index can be max_bits in binary rep
        int delta = max_neg+num;
        return (max_neg | delta);
    }
}

int isHexa (char* str){// check 0xnull case ??
   if (str[0] == '0' && (str[1] == 'x' || str[1] = 'X')) {return strlen(str+2);}
   return 0;
}

int isletter(char c){
    if ((c >= 65 && c<= 90 ) || (c>=97 && c<= 122)) {return 1;}
    return 0;
}

