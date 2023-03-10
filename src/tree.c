#include<tree.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* string values for ast node types, makes tree output more readable */
char *nodeNames[33] = {"program", "declList", "decl", "varDecl", "typeSpecifier",
                       "funDecl", "formalDeclList", "formalDecl", "funBody",
                       "localDeclList", "statementList", "statement", "compoundStmt",
                       "assignStmt", "condStmt", "loopStmt", "returnStmt","expression",
                       "relop", "addExpr", "addop", "term", "mulop", "factor",
                       "funcCallExpr", "argList", "integer", "identifier", "var",
                       "arrayDecl", "char", "funcTypeName"};

char *typeNames[3] = {"int", "char", "void"};
char *ops[10] = {"+", "-", "*", "/", "<", "<=", "==", ">=", ">", "!="};

tree *maketree(int kind) {
      if (DEBUG) {
        printf("maketree() called...\n");
      }
      tree *this = (tree *) malloc(sizeof(struct treenode));
      this->nodeKind = kind;
      this->numChildren = 0;
      if (DEBUG) {
        printf("...maketree() finished\n");
      }
      return this;
}

tree *maketreeWithVal(int kind, int val) {
      if (DEBUG) {
        printf("maketreeWithVal() called...\n");
      }
      tree *this = (tree *) malloc(sizeof(struct treenode));
      this->nodeKind = kind;
      this->numChildren = 0;
      this->val = val;
      if (DEBUG) {
        printf("...maketreeWithVal() finished\n");
      }
      return this;
}

void addChild(tree *parent, tree *child) {
      if (DEBUG) {
        printf("addChild() called...\n");
      }
      if (parent->numChildren == MAXCHILDREN) {
          printf("Cannot add child to parent node\n");
          exit(1);
      }
      nextAvailChild(parent) = child;
      parent->numChildren++;
      if (DEBUG) {
        printf("...addChild() finished\n");
      }
}

int funcTypeNames = 0;
void printAst(tree *node, int nestLevel) {
      char* nodeName = nodeNames[node->nodeKind];
      if(strcmp(nodeName,"identifier") == 0){
          if(node->val == -1) {
            printf("%s,%s\n", nodeName,"undeclared variable");
          }
          else {
            printf("%s,%s\n", nodeName,get_symbol_id(node->val, node->scope));
          }
      }
      else if(strcmp(nodeName,"integer") == 0){
          printf("%s,%d\n", nodeName,node->val);
      }
      else if(strcmp(nodeName,"char") == 0){
          printf("%s,%c\n", nodeName,node->val);
      }
      else if(strcmp(nodeName,"typeSpecifier") == 0){
          printf("%s,%s\n", nodeName,typeNames[node->val]);
      }
      else if(strcmp(nodeName,"relop") == 0 || strcmp(nodeName,"mulop") == 0 || strcmp(nodeName,"addop") == 0){
          printf("%s,%s\n", nodeName,ops[node->val]);
      }
      else if(strcmp(nodeName,"term") == 0){
          printf("%s,%d\n", nodeName,node->val);
      }
      else{
          printf("%s\n", nodeName);
      }

      int i, j;

      for (i = 0; i < node->numChildren; i++)  {
          for (j = 0; j < nestLevel; j++) printf("    ");
          printAst(getChild(node, i), nestLevel + 1);
      }
        
      if (strcmp(nodeName,"funcTypeName") == 0) {
        current_scope = current_scope->first_child;
        for (int i=0; i<funcTypeNames; ++i) {
          current_scope = current_scope->next;
        }
        ++funcTypeNames;
      }
      else if (strcmp(nodeName,"funBody") == 0) {
        current_scope = current_scope->parent;
      }
}

void flattenList(tree *list, tree *subList){
    for(int i=0; i < subList->numChildren; i++) {
        addChild(list,getChild(subList,i));
    }
}

void checkArgList(tree* argList, char* id, int argListVal) {
  symEntry* entry = ST_lookup(id);
  param* firstParam = entry->params;
  param* current_param = firstParam;
  int numOfArguments = 0;
  tree* current_node = argList;

  numOfArguments = current_node->numChildren;

  if (numOfArguments < entry->size) {
    yyerror("Too few arguments provided in function call.");
    printf("Copmilation was aborted.\n");
    exit(-1);
  }
  else if (numOfArguments > entry->size) {
    yyerror("Too many arguments provided in function call.");
    printf("Copmilation was aborted.\n");
    exit(-1);
  }
  else { // same number of arguments as function declaration

    // reverse order of parameters
    param* parameters[numOfArguments];
    int enteredParameters = 0;
    current_node = argList;

    while (enteredParameters < numOfArguments) {
      parameters[numOfArguments - 1 - enteredParameters] = getType(current_node->children[enteredParameters]);
      ++enteredParameters;
    }

    bool printedAnError = false;
    for (int i = numOfArguments - 1; current_param && !printedAnError; --i) {
      if (current_param->symbol_type != parameters[i]->symbol_type ||
          current_param->data_type != parameters[i]->data_type) {
          printedAnError = true;
          yyerror("Argument type mismatch in function call.");
          printf("Copmilation was aborted.\n");
          exit(-1);
      }
      current_param = current_param->next;
    }

  }
}

void checkExpression(tree* expression, tree* var) {
  param* varType = getType(var);
  param* expType = getType(expression);
  if (varType != NULL && expType != NULL) {
    if (expType->symbol_type != varType->symbol_type ||
        expType->data_type != varType->data_type) {
      yyerror("Type mismatch in assignment.");
      printf("Copmilation was aborted.\n");
      exit(-1);
    }
    else if (varType->symbol_type != SCALAR) {
      yyerror("Cannot assign expression to non-scalar variable type.");
      printf("Copmilation was aborted.\n");
      exit(-1);
    }
    else if (expType->symbol_type != SCALAR) {
      yyerror("Cannot assign non-scalar expression to a variable.");
      printf("Copmilation was aborted.\n");
      exit(-1);
    }
  }
}


param* getType(tree* node) {
  param* p = malloc(sizeof(param));
  switch (node->sym_type) {
    case INT_TYPE: {
      p->data_type = INT_TYPE;
      p->symbol_type = SCALAR;
      break;
    }
    case CHAR_TYPE: {
      p->data_type = CHAR_TYPE;
      p->symbol_type = SCALAR;
      break;
    }
    case VOID_TYPE: {
      p->data_type = VOID_TYPE;
      p->symbol_type = SCALAR;
      break;
    }
    case INT_ARRAY_TYPE: {
      p->data_type = INT_TYPE;
      p->symbol_type = ARRAY;
      break;
    }
    case CHAR_ARRAY_TYPE: {
      p->data_type = CHAR_TYPE;
      p->symbol_type = ARRAY;
      break;
    }
    case VOID_ARRAY_TYPE: {
      p->data_type = VOID_TYPE;
      p->symbol_type = ARRAY;
      break;
    }
    case -1: {
      free(p);
      return NULL;
      break;
    }
    default: {
      yyerror("failure in compiler logic");
      printf("Copmilation was aborted.\n");
      exit(-1);
      break;
    }
  }
  return p;
}
