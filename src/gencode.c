#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tree.h>   // definition for nodes of the AST
#include <strtab.h> // the symbol table for needed information about nodes
#include "gencode.h"

// a counter for jump labels to be made unique
int labelcounter=0;
// reset register counter (done after calling a func or at beginning of func)
int registercounter=7;

// defines to get the next label counter or to reset registers
#define nextlabel() (++labelcounter)
#define resetregisters() (registercounter=7)

enum nodeTypes {PROGRAM, DECLLIST, DECL, VARDECL, TYPESPEC, FUNDECL,
                FORMALDECLLIST, FORMALDECL, FUNBODY, LOCALDECLLIST,
                STATEMENTLIST, STATEMENT, COMPOUNDSTMT, ASSIGNSTMT,
                CONDSTMT, LOOPSTMT, RETURNSTMT, EXPRESSION, RELOP,
                ADDEXPR, ADDOP, TERM, MULOP, FACTOR, FUNCCALLEXPR,
                ARGLIST, INTEGER, IDENTIFIER, VAR, ARRAYDECL, CHAR,
                FUNCTYPENAME, ADDNODE};

enum opType {ADD, SUB, MUL, DIV, LT, LTE, EQ, GTE, GT, NEQ};

/*
An example of how / why for label counters

Suppose we have:
FORNODE
FORNODE->children[0] = stmtnode (init)
FORNODE->children[1] = condnode (condition)
FORNODE->children[2] = compoundstmt (body)
FORNODE->children[3] = stmtnode (post)

void emitforstmt(tree *ast,FILE *outfile) {
  // Get a unique label number for the labels of this FORSTMT
  const int forlabel = nextlabel();

  // emit the for statement:
  fprintf(outfile,"FORSTMT_INIT%d: # Preparing to do for loop\n",
                  forlabel);
  gencode(ast->children[0],outfile);
  fprintf(outfile,"FORSTMT_COND%d: # For statement condition\n",
                  forlabel);
  // how you evaluate conditions will probably vary
  const int result = gencode(ast->children[1],outfile);
  // here I am assuming this register will equal zero if the condition is false
  fprintf(outfile,"beq %s, $zero, FORSTMT_END%d # Exit the for statement\n",
                  registernames[result], forlabel);
  // otherwise we do the body of the for loop
  fprintf(outfile,"FORSTMT_BODY%d: # Begin the for statement\'s body\n",
                  forlabel);
  // emit the body of the loop
  gencode(ast->children[2],outfile);
  // do the post expression
  fprintf(outfile,"FORSTMT_POST%d: # For statement post statement\n",
                  forlabel);
  // We need to return to the for loop's condition
  fprintf(outfile,"j FORSTMT_COND%d # Return to for condition\n",
                  forlabel);
  // Now we print the label for the for statement exit
  fprintf(outfile,"FORSTMT_END%d: # End of for loop\n",
                  forlabel);
}*/

// int gencode(tree *ast,FILE *outfile) {
//   int lhs, rhs, dst;
//   switch(ast->nodeKind) {
//   case CHAR:
//     dst = nextreg();
//     fprintf(outfile, "li,%s,%d\n", registernames[dst], ast->val)
//     return dst;
//   case INTEGER:
//     dst = nextreg();
//     fprintf(outfile, "li,%s,%d\n", registernames[dst], ast->val)
//     return dst;
//   case VOID_TYPE:
//         dst = nextreg();
//         fprintf(outfile,"li %s, %d\n", registernames[dst], ast->val);
//         return dst;
//   case :
//     emitforstmt(ast,outfile); return 0;
//   }
//   // if we don't return a value in the switch statement then it wasn't handled
//   // returning -1 should cause a seg-fault if used to index registernames[]
//   return -1;
// }

// void emitfunction(tree *func) {
//     if (strcmp(func->stentry->id, "main") == 0) return;
  
//           if(ast->stentry == NULL)
//               printf("%s,%s\n","undeclared variable"); ////print label for the function body
//           else
//               printf("%s,%s\n", ast->stentry->id);

//     setupframe(); //setup activation record, save registers, move fp & sp

//     resetregisters();

//     int counter = numlocalvar(ast);

//     for(int i = 0; i < counter; ++i) emitfunction();

//     teardownframe_andrestore();

//     if (strcmp(, "main") == 0)
//         fprintf("10");              //syscall_for_clean_exit(); < -10
//     else {
//         fprintf("jr $ra\n"); //put return value into the stack? "jal $ra"
//     }

// } 



int numlocalvar(tree* node) {
  // parameter will be given an offset of 4
  if (node->numChildren == 3) { // there is a parameter
    node->children[1]->children[0]->children[1]->stentry->stackoffset = 4;
  }

  // setting node = to funbody node to look at child node local vars
  if (node->children[1]->nodeKind == FORMALDECLLIST) node = node->children[2];
  else node = node->children[1];
  
  if (node->numChildren == 0) {
    return 0;
  }
  else if (node->children[0]->nodeKind == LOCALDECLLIST) {
    tree* localDeclListNode = node->children[0];
    for (int i=0; i<localDeclListNode->numChildren; ++i) {
      localDeclListNode->children[i]->children[1]->stentry->stackoffset = 4 * i + 8;
      // first local var will have an offset of 8, second 12 and so on
    }
    return localDeclListNode->numChildren;
  }
  else { // no localdecllist child
    return 0;
  }
}

int counter; // global so it can accessed in tear down frame and restore
void setupframe(tree* ast, FILE* outfile) {
  counter = numlocalvar(ast);
  
  // for return address / address of return result and optionally a parameter
  counter += 3;
  
  fprintf(outfile, "\tsw, $fp, 0($sp)\n");        //# prev fp
  fprintf(outfile, "\tsw, $ra, -4($sp)\n");        //# prev ra
  fprintf(outfile, "\taddu, $fp, $zero, $sp\n");    //# curr fp
  fprintf(outfile ,"\taddiu, $sp, $sp, %d\n", -4 * counter); //stack frame size in bytes
  //stackoffset = ;
  return;
}

void teardownframe_andrestore(tree* ast, FILE* outfile) {
  fprintf(outfile, "\taddiu $sp, $sp, %d	# restore sp, exiting function\n", 4 * counter);
  fprintf(outfile, "\tlw $fp, 0($sp)\n");
  fprintf(outfile, "\tlw $ra, -4($sp)\n");

  resetregisters();
  return;
}

void setreturnvalue() {
  // Still need to find out what this does
}

void syscall_for_clean_exit(FILE* outfile) {
  fprintf(outfile, "\tli $v0, 10\n\tsyscall\n");
}

tree* findMainFunction(tree* ast) {
  ast = ast->children[0];
  for(int i=0; i<ast->numChildren; ++i) {
    if (ast->children[i]->nodeKind == FUNDECL) {
      tree* funDeclNode = ast->children[i];
      int idVal = funDeclNode->children[0]->children[1]->val;
      if (strcmp(get_symbol_id(idVal, 0), "main") == 0) {
        return ast->children[i];
      }
    }
  }
  return NULL;
}

void outputAllFunctions(tree* ast, FILE* outfile) {
  ast = ast->children[0];
  for(int i = 0; i < ast->numChildren; ++i) {
    if (ast->children[i]->nodeKind == FUNDECL) {
      tree* funDeclNode = ast->children[i];
      int idVal = funDeclNode->children[0]->children[1]->val;
      char* idName = get_symbol_id(idVal, 0);
      if (strcmp(idName, "main") != 0) {
        fprintf(outfile, "start_%s:\n", idName);
        gencode(ast->children[i], outfile);
      }
    }
  }
}

/*
Number  | Name      | Description
0       | $zero     | The value 0
2-3     | $v0 - $v1 | (values) from expression evaluation and function results
4-7     | $a0 - $a3 | (arguments) First four parameters for subroutine
8-15    | $t0 - $t7 | Temporary variables
16-23   | $s0 - $s7 | Saved values representing final computed results
24-25   | $t8 - $t9 | Temporary variables2
31      | $ra       |Return address
*/
char *registernames[] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                          "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                          "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                          "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra" };
// get the next available register
// this naiively just cycles through $t0 -> $t9 -> $t0
int nextreg() {
  // jump the gap from $t7 to $t8
  if (++registercounter == 16) registercounter = 24;
  // wrap from $t9 to $t0
  else if (registercounter == 26) registercounter = 8;
  return registercounter;
}

int emit(tree* ast) {

    int lhs, rhs, dst;

    switch (ast->nodeKind) {
    case ADDNODE:
        lhs = emit(ast->children[0]);
        rhs = emit(ast->children[1]);
        dst = nextreg();
        printf("\tadd %s, %s %s  # addop\n", registernames[dst], registernames[lhs], registernames[rhs]);
        return dst;
    case ASSIGNSTMT: // assignment statement has lhs = some var and rhs = some expression
        rhs = emit(ast->children[1]); // get the value
        dst = nextreg();
        printf("\tmove %s, %s  # assign variable %s\n", registernames[dst], registernames[rhs], ast->stentry->id);
        // if we immediately move this somewhere into the stack ...
        printf("\tsw %s, %d($fp)  # store %s into the stack\n", registernames[dst], ast->stentry->stackoffset, ast->stentry->id);
        return dst;
    }
    return 0;
}

/*
 * The following page is a good reference for MIPS
https://ecs-network.serv.pacific.edu/ecpe-170/tutorials/mips-instruction-set
*/
char* currentFuncName;
int gencode(tree *ast,FILE *outfile) {
  // variables can't be defined inside a switch
  // move more complicated things to their own function to keep this cleaner
  // (e.g., function definition needs to setup/teardown stack, ...)
  int lhs,rhs,dst;
  switch(ast->nodeKind) {
    case PROGRAM: {
      fprintf(outfile, "\t.data\n");
      printGlobalVars(outfile);
      fprintf(outfile, "\n\t.text\n\t.globl main\n");
      tree* mainFunction = findMainFunction(ast);
      if (mainFunction == NULL) {
        fprintf(outfile, "ERROR: there is no main function\nCompilation aborted.\n");
        exit(-1);
      }
      else {
        fprintf(outfile, "main:\n");
      }
      gencode(mainFunction, outfile);
      outputAllFunctions(ast, outfile);
      break;
    }
    case CHAR: {
      dst = nextreg();
      fprintf(outfile, "\tli,%s,%d\n", registernames[dst], ast->val);
      return dst;
      break;
    }
    case INTEGER: {
      dst = nextreg();
      fprintf(outfile,"\tli %s, %d\n",registernames[dst], ast->val);
      return dst;
      break;
    }
    case IDENTIFIER: {
      int reg = nextreg();
      if (ast->scope == 0) { // if it's in global scope
        fprintf(outfile, "\tla $a1, st%d\n", ast->stentry->index);
        fprintf(outfile, "\tlw %s, 0($a1)\n", registernames[reg]);
      }
      else { // if it's a local variable
        fprintf(outfile, "\tlw %s, %d($sp)\n", registernames[reg], ast->stentry->stackoffset);
      }
      return reg;
      break;
    }
    case FUNDECL: {
      currentFuncName = ast->children[0]->children[1]->stentry->id;
      resetregisters();
      setupframe(ast, outfile);
      
      //if there is a parameter store it
      if (ast->numChildren == 3) {
        fprintf(outfile, "\tsw $a0, %d($sp)\n", ast->children[1]->children[0]->children[1]->stentry->stackoffset);
      }
      
      // for all statements in funcdefnode emitcode();
      tree* funBodyNode;
      if (ast->numChildren == 2) {
        funBodyNode = ast->children[1];
      }
      else { // numChildren == 3
        funBodyNode = ast->children[2];
      }
      tree* statementListNode;
      if (funBodyNode->numChildren == 0) {
        // no statements to emit - making an empty statement list node with no children
        statementListNode = maketree(STATEMENTLIST);
      }
      else if (funBodyNode->numChildren == 1) {
        if (funBodyNode->children[0]->nodeKind == STATEMENTLIST) {
          // statement list node that has no localdecllist sibling
          statementListNode = funBodyNode->children[0];
        }
        else {
          // localdecllist node with no statementlist node sibling
          statementListNode = maketree(STATEMENTLIST);
        }
      }
      else { // 2 children - 2nd one is the statement list
        statementListNode = funBodyNode->children[1];
      }
      for (int i = 0; i < statementListNode->numChildren; ++i) {
        gencode(statementListNode->children[i], outfile);
      }
      
      
      char* idName = get_symbol_id(ast->children[0]->children[1]->val, 0);
      fprintf(outfile, "end_%s:\n", idName);
      teardownframe_andrestore(ast, outfile);
      setreturnvalue();
      int func_id_val = ast->children[0]->children[1]->val;
      int func_id_scope = ast->children[0]->children[1]->scope;
      if (strcmp(get_symbol_id(func_id_val, func_id_scope),"main")==0)
        syscall_for_clean_exit(outfile);
      else {
        fprintf(outfile,"\tjr $ra\n");
      }
      return 0; // should return something here?
      break;
    }
    case FUNCCALLEXPR: {
      int func_id_val = ast->children[0]->val;
      int func_id_scope = ast->children[0]->scope;
      if (strcmp(get_symbol_id(func_id_val, func_id_scope),"output") == 0) {
        rhs = gencode(ast->children[1]->children[0], outfile);
        fprintf(outfile, "\tmove $a0, %s\n",registernames[rhs]);
        fprintf(outfile, "\tli $v0, 1\n");
        fprintf(outfile, "\tsyscall\n");
        fprintf(outfile, "\tli $v0, 11\n");
        fprintf(outfile, "\tli $a0, '\\n'\n");
        fprintf(outfile, "\tsyscall\n");
      }
      else {
        if (ast->numChildren == 2) { // there is an argument
          int arg = gencode(ast->children[1]->children[0], outfile);
          fprintf(outfile, "\tmove $a0, %s\n", registernames[arg]);
        }
        fprintf(outfile, "\tjal start_%s\n", ast->children[0]->stentry->id);
      
        // save registers, prepare to move context
        // resolve arguments, put them where appropriate
        // jump and link to function label
        // immediately after jump and link all $t_ registers are garbage.
        // return value will be (somewhere)
        resetregisters();
        // anything needed will have to be restored from the stack
      }
      // return 0, break, (?)
      return 2; // return value will always be stored in $v0 register which is register 2
      break;
    }
    case RETURNSTMT: {
      if (ast->numChildren == 1) { // actually returning something
        int returnValueReg = gencode(ast->children[0], outfile);
        fprintf(outfile, "\tmove $v0, %s\n", registernames[returnValueReg]); // put return value in $v0 register
      }
      // jump to end of function to restore stack/frame pointers
      fprintf(outfile, "\tj end_%s\n", currentFuncName);
      return 2; // return $v0 register (default return register)
      break;
    }
    case ADDOP: {
      lhs = gencode(ast->children[0], outfile);
      rhs = gencode(ast->children[1], outfile);
      dst = rhs;
      if (ast->val == ADD) {
        fprintf(outfile, "\tadd %s, %s, %s  # addop\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else { // subtraction
        fprintf(outfile, "\tsub %s, %s, %s  # subop\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      return dst;
      break;
    }
    case MULOP: {
      lhs = gencode(ast->children[0], outfile);
      rhs = gencode(ast->children[1], outfile);
      dst = rhs;
      if (ast->val == MUL) {
        fprintf(outfile, "\tmul %s, %s, %s  # mulop\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else { // division
        fprintf(outfile, "\tdiv %s, %s  # divop\n", registernames[lhs], registernames[rhs]);
        fprintf(outfile, "\tmflo %s\n", registernames[dst]);
      }
      return dst;
      break;
    }
    case ASSIGNSTMT: {
      rhs = gencode(ast->children[1], outfile);
      if (ast->children[0]->scope == 0) { // if assigning to global
        fprintf(outfile, "\tla $a1, st%d\n", ast->children[0]->stentry->index);
        fprintf(outfile, "\tsw %s, 0($a1)\n", registernames[rhs]);
      }
      else { // if assigning to local
        int offset = ast->children[0]->stentry->stackoffset;
        fprintf(outfile, "\tsw %s, %d($sp)\n", registernames[rhs], offset);
      }
      return 0;
      break;
    }
    case COMPOUNDSTMT: {
      for (int i=0; i<ast->numChildren; ++i) {
        gencode(ast->children[i], outfile);
      }
      return 0;
      break;
    }
    case STATEMENTLIST: {
      for (int i=0; i<ast->numChildren; ++i) {
        gencode(ast->children[i], outfile);
      }
      return 0;
      break;
    }
    case CONDSTMT: {
      int label = nextlabel();
      int expression = gencode(ast->children[0], outfile);
      fprintf(outfile, "\tbeq %s, $zero, condition%d\n", registernames[expression], label);
      gencode(ast->children[1], outfile);
      if (ast->numChildren == 3) { // if there is an else statement
        fprintf(outfile, "\tj condition%d\n", label + 1);
      }
      fprintf(outfile, "condition%d:\n", label);
      if (ast->numChildren == 3) { // if there is an else statement
        gencode(ast->children[2], outfile);
        fprintf(outfile, "condition%d:\n", nextlabel());
      }
      return 0;
      break;
    }
    case LOOPSTMT: {
      int label = nextlabel();
      fprintf(outfile, "condition%d:\n", label); //start of loop
      int expression = gencode(ast->children[0], outfile);
      fprintf(outfile, "\tbeq %s, $zero, condition%d\n", registernames[expression], label + 1); //if fail branch to end
      gencode(ast->children[1], outfile);
      fprintf(outfile, "\tj condition%d\n", label); // j to top
      fprintf(outfile, "condition%d:\n", nextlabel()); // end of loop
      return 0;
      break;
    }
    case RELOP: {
      int lhs = gencode(ast->children[0], outfile);
      int rhs = gencode(ast->children[1], outfile);
      int dst = rhs;
      if (ast->val == LT) {
        fprintf(outfile, "\tslt %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else if (ast->val == LTE) {
        fprintf(outfile, "\tsle %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else if (ast->val == EQ) {
        fprintf(outfile, "\tseq %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else if (ast->val == GTE) {
        fprintf(outfile, "\tsge %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else if (ast->val == GT) {
        fprintf(outfile, "\tsgt %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      else { // NEQ
        fprintf(outfile, "\tsne %s, %s, %s\n", registernames[dst], registernames[lhs], registernames[rhs]);
      }
      return dst;
      break;
    }
    default: { break; }
  }
  return -1; // (?)
}
