int read_from_file(FILE *fp, int len, Mode mode) {
    char *line = malloc(len*sizeof(char));
    int res;
    switch (mode) {
        case (0): //Read dmemin
            res = init_data_lst(fp, line, len);
            break;
        case (1): //Read imemin
            res = init_cmd_lst(fp, line, len);
            break;
        case (2): //Read irq2in
            res = init_irq2_lst(fp, line, len);
            break;
    }
    fill_with_null(res, MAX_INSTRUCTIONS, mode);
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

