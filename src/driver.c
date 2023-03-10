#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<../src/tree.h>
#include<../src/strtab.h>
#include<../src/gencode.h>

extern FILE* yyin;
extern int yyparse();

void printhelp(){
  printf("Usage: mcc [--ast] [--sym] [-o|--output FILE] [-h|--help] FILE\n");
  printf("\t--ast:\t\tPrint a textual representation of the constructed abstract syntax tree.\n");
  printf("\t--sym:\t\tPrint a textual representation of the constructed symbol table.\n");
  printf("\t-h,--help:\tPrint this help information and exit.\n\n");
}

int main(int argc, char *argv[]) {
  int p_ast = 0;
  int p_symtab = 0;

  FILE *outfile=NULL;

  // Skip first arg (program name), then check all but last for options.
  for(int i=1; i < argc - 1; i++){
    if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0){
      printhelp();
      return 0;
    }
    else if(strcmp(argv[i],"--ast")==0){
      p_ast = 1;
    }
    else if(strcmp(argv[i],"--sym")==0){
      p_symtab = 1;
    }
    else if (strcmp(argv[i],"-o")==0 || strcmp(argv[i],"--output")==0) {
      if (++i < argc) outfile=fopen(argv[i],"w");
      else {
        printhelp();
        return 0;
      }
    }
    else{
      printhelp();
      return 0;
    }
  }
  // if we didn't open an output file
  if (!outfile) outfile=stdout;

  // if no file was specified (no arguments) or can't open last argument
  if (argc == 1 || (yyin = fopen(argv[argc - 1],"r")) == NULL)
    yyin = stdin;

  if (!yyparse()){
    printf("Compilation finished.\n\n");
    if(p_ast)
      printAst(ast, 1);
    if(p_symtab) {
      print_sym_tab("GLOBAL");
      current_scope = current_scope->first_child;
      while (current_scope) {
        print_sym_tab("FUNCTION");
        current_scope = current_scope->next;
      }
    }
    // pass the AST and FILE pointer to gencode() to print generated asm
    gencode(ast,outfile);
  }
  return 0;
}
