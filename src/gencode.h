
int gencode(tree* ast, FILE* outfile);
int nextreg();
void setupframe(tree* ast, FILE* outfile);
void teardownframe_andrestore(tree* ast, FILE* outfile);
void setreturnvalue();
void syscall_for_clean_exit(FILE*);
tree* findMainFunction(tree*);
void outputAllFunctions(tree*, FILE*);
