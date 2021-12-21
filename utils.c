int get_current_dir(char *cwd) {
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   } else {
       fprintf(stderr, "getcwd() error");
       return 1;
   }
   return 0;
}

char cut_string_by_index(char *str, int i) {
	char tmp = str[i];
    str[i] = '\0';
    return tmp;
}


