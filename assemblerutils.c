#include "Assembler.h"


//Utils Func Declarations
int compare (const void * a, const void * b);
void sign_ext(int *num);
void dec2hexa(char* result, int num, int len);
int hexa2dec(char *hex_rep, int len);
int get_max(int a, int b);

//Assembler Related Func
int init_unparsed_instructions(FILE *fp, int len);


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


int init_unparsed_instructions(FILE *fp, int len) {
    char *line = malloc(len*sizeof(char));
    int lines_read = 0;
    while(fgets(line, len, fp) != NULL) {
        line[strcspn(line, "\n")] = '\0'; //replacing \n (if exists) in buffer with null-terminating
        strcpy(unparsed_instructions[lines_read], line);
        int newlineSize = (strlen(unparsed_instructions[lines_read])+1)*sizeof(char);
        realloc(unparsed_instructions[lines_read], newlineSize);
        lines_read++;
    }
    free(line);
    return lines_read;
}