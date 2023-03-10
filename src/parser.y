%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<../src/tree.h>
#include<../src/strtab.h>
#include <stdbool.h>

//silences a bunch of implicit declaration warnings
extern int yylineno;
extern int yyerror();
extern int yywarning(char*);
extern int yylex();

/* nodeTypes refer to different types of internal and external nodes that can be part of the abstract syntax tree.*/
enum nodeTypes {PROGRAM, DECLLIST, DECL, VARDECL, TYPESPEC, FUNDECL,
                FORMALDECLLIST, FORMALDECL, FUNBODY, LOCALDECLLIST,
                STATEMENTLIST, STATEMENT, COMPOUNDSTMT, ASSIGNSTMT,
                CONDSTMT, LOOPSTMT, RETURNSTMT, EXPRESSION, RELOP,
                ADDEXPR, ADDOP, TERM, MULOP, FACTOR, FUNCCALLEXPR,
                ARGLIST, INTEGER, IDENTIFIER, VAR, ARRAYDECL, CHAR,
                FUNCTYPENAME};

enum opType {ADD, SUB, MUL, DIV, LT, LTE, EQ, GTE, GT, NEQ};

/* NOTE: mC has two kinds of scopes for variables : local and global. Variables declared outside any
function are considered globals, whereas variables (and parameters) declared inside a function foo are local to foo. You should update the scope variable whenever you are inside a production that matches function definition (funDecl production). The rationale is that you are entering that function, so all variables, arrays, and other functions should be within this scope. You should pass this variable whenever you are calling the ST_insert or ST_lookup functions. This variable should be updated to scope = "" to indicate global scope whenever funDecl finishes. Treat these hints as helpful directions only. You may implement all of the functions as you like and not adhere to my instructions. As long as the directory structure is correct and the file names are correct, we are okay with it. */
char* scope = "";
int index_of_function_id_in_global_scope = -1;
%}

/* the union describes the fields available in the yylval variable */
%union
{
    int value;
    struct treenode *node;
    char *strval;
}

/*Add token declarations below. The type <value> indicates that the associated token will be of a value type such as integer, float etc., and <strval> indicates that the associated token will be of string type.*/
%token <strval> ID
%token <value> INTCONST
/* TODO: Add the rest of the tokens below.*/
%token <strval> KWD_IF
%token <strval> KWD_ELSE
%token <strval> KWD_WHILE
%token <strval> KWD_INT
%token <strval> KWD_STRING
%token <strval> KWD_CHAR
%token <strval> KWD_RETURN
%token <strval> KWD_VOID
%token <strval> OPER_ADD
%token <strval> OPER_SUB
%token <strval> OPER_MUL
%token <strval> OPER_DIV
%token <strval> OPER_LTE
%token <strval> OPER_GTE
%token <strval> OPER_LT
%token <strval> OPER_GT
%token <strval> OPER_EQ
%token <strval> OPER_NEQ
%token <strval> OPER_ASGN
%token <strval> LSQ_BRKT
%token <strval> RSQ_BRKT
%token <strval> LCRLY_BRKT
%token <strval> RCRLY_BRKT
%token <strval> LPAREN
%token <strval> RPAREN
%token <strval> COMMA
%token <strval> SEMICLN
%token <value> CHARCONST
%token <strval> STRCONST
%token <strval> ILLEGAL_TOKEN
%token <strval> ERROR



/* TODO: Declate non-terminal symbols as of type node. Provided below is one example. node is defined as 'struct treenode *node' in the above union data structure. This declaration indicates to parser that these non-terminal variables will be implemented using a 'treenode *' type data structure. Hence, the circles you draw when drawing a parse tree, the following lines are telling yacc that these will eventually become circles in an AST. This is one of the connections between the AST you draw by hand and how yacc implements code to concretize that. We provide with two examples: program and declList from the grammar. Make sure to add the rest.  */
%type <node> program
%type <node> declList
%type <node> decl
%type <node> varDecl
%type <node> typeSpecifier
%type <node> funDecl
%type <node> formalDeclList
%type <node> formalDecl
%type <node> funBody
%type <node> localDeclList
%type <node> statementList
%type <node> statement
%type <node> compoundStmt
%type <node> assignStmt
%type <node> condStmt
%type <node> loopStmt
%type <node> returnStmt
%type <node> var
%type <node> expression
%type <node> relop
%type <node> addExpr
%type <node> addop
%type <node> term
%type <node> mulop
%type <node> factor
%type <node> funcCallExpr
%type <node> argList
%type <node> funcTypeName
%type <node> arrayDecl

/**/
%start program

%%
/* TODO: Your grammar and semantic actions go here. We provide with two example productions and their associated code for adding non-terminals to the AST.*/

program         : {new_scope();
                   symEntry* outputEntry = ST_insert("output", VOID_TYPE, FUNCTION, "", 1);
                   current_scope->strTable[outputEntry->index]->params = malloc(sizeof(param));
                   current_scope->strTable[outputEntry->index]->params->next = NULL;
                   current_scope->strTable[outputEntry->index]->params->data_type = INT_TYPE;
                   current_scope->strTable[outputEntry->index]->params->symbol_type = SCALAR;
                  } declList
                 {
                    tree* progNode = maketree(PROGRAM);
                    addChild(progNode, $2);
                    ast = progNode;
                 }
                ;

declList        : decl
                 {
                    $$ = maketree(DECLLIST);
                    addChild($$, $1);
                 }
                | declList decl
                 {
                    $$ = $1;
                    addChild($$, $2);
                 }
                ;

decl            : varDecl
                 {
                    $$ = $1;
                 }
                | funDecl
                 {
                    $$ = $1;
                 }
                ;
                
varDecl         : typeSpecifier ID LSQ_BRKT INTCONST RSQ_BRKT SEMICLN
                 {
                    tree* varDeclNode = maketree(VARDECL);
                    symEntry* entry = ST_insert($2, ($1)->val, ARRAY, scope, $4);
                    if (entry == NULL) {
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if ($4 == 0) {
                      yyerror("Array variable declared with size of zero.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    tree* idNode = maketreeWithVal(IDENTIFIER, entry->index);
                    idNode->scope = strcmp(scope,"") == 0 ? 0 : 1;
                    idNode->type = $1->val;
                    idNode->stentry = entry;
                    tree* integerNode = maketreeWithVal(INTEGER, $4);
                    addChild(varDeclNode, $1);
                    addChild(varDeclNode, idNode);
                    addChild(varDeclNode, integerNode);
                    $$ = varDeclNode;
                 }
                | typeSpecifier ID SEMICLN
                 {
                    tree* varDeclNode = maketree(VARDECL);
                    symEntry* entry = ST_insert($2, ($1)->val, SCALAR, scope, 0);
                    if (entry == NULL) {
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    tree* idNode = maketreeWithVal(IDENTIFIER, entry->index);
                    idNode->scope = strcmp(scope,"") == 0 ? 0 : 1;
                    idNode->type = $1->val;
                    idNode->stentry = entry;
                    addChild(varDeclNode, $1);
                    addChild(varDeclNode, idNode);
                    $$ = varDeclNode;
                 }
                ;
                
typeSpecifier   : KWD_INT
                 {
                    if (DEBUG) printf("typespecifier\n");
                    tree* typeSpecifierNode = maketreeWithVal(TYPESPEC, INT_TYPE);
                    $$ = typeSpecifierNode;
                 }
                | KWD_CHAR
                 {
                    if (DEBUG) printf("typespecifier\n");
                    tree* typeSpecifierNode = maketreeWithVal(TYPESPEC, CHAR_TYPE);
                    $$ = typeSpecifierNode;
                 }
                | KWD_VOID
                 {
                    if (DEBUG) printf("typespecifier\n");
                    tree* typeSpecifierNode = maketreeWithVal(TYPESPEC, VOID_TYPE);
                    $$ = typeSpecifierNode;
                 }
                ;

funDecl         : funcTypeName LPAREN formalDeclList RPAREN funBody
                 {
                    tree* funDeclNode = maketree(FUNDECL);
                    addChild(funDeclNode, $1);
                    addChild(funDeclNode, $3);
                    addChild(funDeclNode, $5);
                    $$ = funDeclNode;
                    up_scope();
                    scope = "";
                    index_of_function_id_in_global_scope = -1;
                 }
                | funcTypeName LPAREN RPAREN funBody
                 {
                    tree* funDeclNode = maketree(FUNDECL);
                    addChild(funDeclNode, $1);
                    addChild(funDeclNode, $4);
                    $$ = funDeclNode;
                    up_scope();
                    scope = "";
                    index_of_function_id_in_global_scope = -1;
                 }
                ;
        
funcTypeName    : typeSpecifier ID
                 {
                    tree* funcTypeNameNode = maketree(FUNCTYPENAME);
                    symEntry* entry = ST_insert($2, ($1)->val, FUNCTION, scope, 0);
                    if (entry == NULL) {
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    tree* idNode = maketreeWithVal(IDENTIFIER, entry->index);
                    idNode->scope = strcmp(scope,"") == 0 ? 0 : 1;
                    idNode->stentry = entry;
                    addChild(funcTypeNameNode, $1);
                    addChild(funcTypeNameNode, idNode);
                    $$ = funcTypeNameNode;
                    scope = $2;
                    new_scope();
                    index_of_function_id_in_global_scope = entry->index;
                 }
                ;
                
formalDeclList  : formalDecl
                 {
                    $$ = maketree(FORMALDECLLIST);
                    addChild($$, $1);
                    ++current_scope->parent->strTable[index_of_function_id_in_global_scope]->size;
                 }
                | formalDeclList COMMA formalDecl
                 {
                    $$ = $1;
                    addChild($$, $3);
                    ++current_scope->parent->strTable[index_of_function_id_in_global_scope]->size;
                 }
                ;
                
formalDecl      : typeSpecifier ID
                 {
                    $$ = maketree(FORMALDECL);
                    symEntry* entry = ST_insert($2, ($1)->val, SCALAR, scope, 0);
                    if (entry == NULL) {
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    addParameter(index_of_function_id_in_global_scope, $1->val, SCALAR);
                    tree* idNode = maketreeWithVal(IDENTIFIER, entry->index);
                    idNode->stentry = entry;
                    idNode->scope = strcmp(scope,"") == 0 ? 0 : 1;
                    addChild($$, $1);
                    addChild($$, idNode);
                 }
                | typeSpecifier ID arrayDecl
                 {
                    $$ = maketree(FORMALDECL);
                    symEntry* entry = ST_insert($2, ($1)->val, ARRAY, scope, -1);
                    if (entry == NULL) {
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    addParameter(index_of_function_id_in_global_scope, $1->val, ARRAY);
                    tree* idNode = maketreeWithVal(IDENTIFIER, entry->index);
                    idNode->scope = strcmp(scope,"") == 0 ? 0 : 1;
                    idNode->stentry = entry;
                    addChild($$, $1);
                    addChild($$, idNode);
                    addChild($$, $3);
                 }
                ;
                
arrayDecl       : LSQ_BRKT RSQ_BRKT
                 {
                    $$ = maketree(ARRAYDECL);
                 }
                ;
                
funBody         : LCRLY_BRKT RCRLY_BRKT
                 {
                    $$ = maketree(FUNBODY);
                 }
                 | LCRLY_BRKT localDeclList RCRLY_BRKT
                 {
                    $$ = maketree(FUNBODY);
                    addChild($$, $2);
                 }
                 | LCRLY_BRKT statementList RCRLY_BRKT
                 {
                    $$ = maketree(FUNBODY);
                    addChild($$, $2);
                 }
                 | LCRLY_BRKT localDeclList statementList RCRLY_BRKT
                 {
                    $$ = maketree(FUNBODY);
                    addChild($$, $2);
                    addChild($$, $3);
                 }
                ;
              
localDeclList   : varDecl
                 {
                    $$ = maketree(LOCALDECLLIST);
                    addChild($$, $1);
                 }
                | localDeclList varDecl
                 {
                    $$ = $1;
                    addChild($$, $2);
                 }
                ;
                
statementList   : statement
                 {
                    $$ = maketree(STATEMENTLIST);
                    addChild($$, $1);
                 }
                | statementList statement
                 {
                    $$ = $1;
                    addChild($$, $2);
                 }
                ;
                
statement       : compoundStmt
                 {
                    $$ = $1;
                 }
                | assignStmt
                 {
                    $$ = $1;
                 }
                | condStmt
                 {
                    $$ = $1;
                 }
                | loopStmt
                 {
                    $$ = $1;
                 }
                | returnStmt
                 {
                    $$ = $1;
                 }
                ;
                
compoundStmt    : LCRLY_BRKT statementList RCRLY_BRKT
                 {
                    $$ = $2;
                 }
                ;
                
assignStmt      : var OPER_ASGN expression SEMICLN
                 {
                    $$ = maketree(ASSIGNSTMT);
                    checkExpression($3, $1);
                    addChild($$, $1);
                    addChild($$, $3);
                 }
                | expression SEMICLN
                 {
                    $$ = $1;
                 }
                ;
                
condStmt        : KWD_IF LPAREN expression RPAREN statement
                 {
                    $$ = maketree(CONDSTMT);
                    addChild($$, $3);
                    addChild($$, $5);
                 }
                | KWD_IF LPAREN expression RPAREN statement KWD_ELSE statement
                 {
                    $$ = maketree(CONDSTMT);
                    addChild($$, $3);
                    addChild($$, $5);
                    addChild($$, $7);
                 }
                ;
                
loopStmt        : KWD_WHILE LPAREN expression RPAREN statement
                 {
                    $$ = maketree(LOOPSTMT);
                    addChild($$, $3);
                    addChild($$, $5);
                 }
                ;
              
returnStmt      : KWD_RETURN SEMICLN
                 {
                    $$ = maketree(RETURNSTMT);
                 }
                | KWD_RETURN expression SEMICLN
                 {
                    $$ = maketree(RETURNSTMT);
                    addChild($$, $2);
                 }
                ;
                
var             : ID
                 {
                    symEntry* declaration = ST_lookup($1);
                    if (declaration == NULL) {
                      yyerror("Undeclared variable");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    $$ = maketreeWithVal(IDENTIFIER, declaration->index);
                    if (declaration->symbol_type == ARRAY) {
                      $$->sym_type = declaration->data_type + 3;
                    }
                    else {
                      $$->sym_type = declaration->data_type;
                    }
                    $$->scope = strcmp(declaration->scope,"") == 0 ? 0 : 1;
                    $$->stentry = declaration;
                 }
                | ID LSQ_BRKT addExpr RSQ_BRKT
                 {
                    symEntry* declaration = ST_lookup($1);
                    if (declaration == NULL) {
                      yyerror("Undeclared variable");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (declaration->symbol_type != ARRAY) {
                      yyerror("Non-array identifier used as an array.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (($3)->nodeKind != INTEGER && ($3)->sym_type != INT_TYPE) {
                      yyerror("Array indexed using non-integer expression.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (($3)->nodeKind == INTEGER && ($3)->val < 0) {
                      yyerror("Array indexed with negative, out-of-bounds expression.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    // declaration->size == -1 when array size is unkown (like in a function
                    // parameter) in which size is unkown and considered infinite
                    if (declaration->size != -1 && ($3)->nodeKind == INTEGER && ($3)->val >= declaration->size) {
                      yyerror("Statically sized array indexed with constant, out-of-bounds expression.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    $$ = maketree(VAR);
                    $$->sym_type = declaration->data_type;
                    tree* idNode = maketreeWithVal(IDENTIFIER, declaration->index);
                    idNode->stentry = declaration;
                    idNode->scope = strcmp(declaration->scope,"") == 0 ? 0 : 1;
                    addChild($$, idNode);
                    addChild($$, $3);
                 }
                ;
                
expression      : addExpr
                 {
                    $$ = $1;
                 }
                | expression relop addExpr
                 {
                    // relop expression will always go to an integer (0 or 1)
                    $$ = $2;
                    $$->sym_type = INT_TYPE;
                    addChild($$, $1);
                    addChild($$, $3);
                 }
                ;
                
relop           : OPER_LTE
                 {
                    $$ = maketreeWithVal(RELOP, LTE);
                 }
                | OPER_LT
                 {
                    $$ = maketreeWithVal(RELOP, LT);
                 }
                | OPER_GT
                 {
                    $$ = maketreeWithVal(RELOP, GT);
                 }
                | OPER_GTE
                 {
                    $$ = maketreeWithVal(RELOP, GTE);
                 }
                | OPER_EQ
                 {
                    $$ = maketreeWithVal(RELOP, EQ);
                 }
                | OPER_NEQ
                 {
                    $$ = maketreeWithVal(RELOP, NEQ);
                 }
                ;
                
addExpr         : term
                 {
                    $$ = $1;
                 }
                | addExpr addop term
                 {
                    if ($1->nodeKind != INTEGER && $1->sym_type > VOID_TYPE ||
                        $3->nodeKind != INTEGER && $3->sym_type > VOID_TYPE) {
                      yyerror("Trying to do math with an un-indexed array.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (($1)->nodeKind == INTEGER) {
                      if (($3)->nodeKind == INTEGER) {
                        if (($2)->val == ADD) {
                          $$ = maketreeWithVal(INTEGER, ($1)->val + ($3)->val);
                        }
                        else {
                          $$ = maketreeWithVal(INTEGER, ($1)->val - ($3)->val);
                        }
                      }
                      else if (($3)->sym_type == INT_TYPE) {
                        $$ = $2;
                        $$->sym_type = INT_TYPE;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else {
                        yyerror("Addop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                    else if (($3)->nodeKind == INTEGER) {
                      if (($1)->sym_type == INT_TYPE) {
                        $$ = $2;
                        $$->sym_type = INT_TYPE;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else {
                        yyerror("Addop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                    else { // neither are integers
                      if (($1)->sym_type == ($3)->sym_type) {
                        $$ = $2;
                        $$->sym_type = $1->sym_type;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else { // the types are not the same
                        yyerror("Addop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                 }
                ;
                
addop           : OPER_ADD
                 {
                    $$ = maketreeWithVal(ADDOP, ADD);
                 }
                | OPER_SUB
                 {
                    $$ = maketreeWithVal(ADDOP, SUB);
                 }
                ;
                
term            : factor
                 {
                    $$ = $1;
                 }
                | term mulop factor
                 {
                    if ($1->nodeKind != INTEGER && $1->sym_type > VOID_TYPE ||
                        $3->nodeKind != INTEGER && $3->sym_type > VOID_TYPE) {
                      yyerror("Trying to do math with an un-indexed array.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (($1)->nodeKind == INTEGER) {
                      if (($3)->nodeKind == INTEGER) {
                        if (($2)->val == MUL) {
                          $$ = maketreeWithVal(INTEGER, ($1)->val * ($3)->val);
                        }
                        else {
                          $$ = maketreeWithVal(INTEGER, ($1)->val / ($3)->val);
                        }
                      }
                      else if (($3)->sym_type == INT_TYPE) {
                        $$ = $2;
                        $$->sym_type = INT_TYPE;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else {
                        yyerror("Mulop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                    else if (($3)->nodeKind == INTEGER) {
                      if (($1)->sym_type == INT_TYPE) {
                        $$ = $2;
                        $$->sym_type = INT_TYPE;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else {
                        yyerror("Mulop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                    else {
                      if (($1)->sym_type == ($3)->sym_type) { // math with same types
                        $$ = $2;
                        $$->sym_type = $1->sym_type;
                        addChild($$, $1);
                        addChild($$, $3);
                      }
                      else {
                        yyerror("Mulop with mismatch types.");
                        printf("Compilation was aborted.\n");
                        YYABORT;
                      }
                    }
                 }
                ;
                
mulop           : OPER_MUL
                 {
                    $$ = maketreeWithVal(MULOP, MUL);
                 }
                | OPER_DIV
                 {
                    $$ = maketreeWithVal(MULOP, DIV);
                 }
                ;
                
factor          : LPAREN expression RPAREN { $$ = $2; }
                | var { $$ = $1; }
                | funcCallExpr { $$ = $1; }
                | INTCONST
                 {
                    $$ = maketreeWithVal(INTEGER, $1);
                 }
                | CHARCONST
                 {
                    $$ = maketree(FACTOR);
                    $$->sym_type = CHAR_TYPE;
                    tree* charNode = maketreeWithVal(CHAR, $1);
                    addChild($$, charNode);
                 }
                ;
                
funcCallExpr    : ID LPAREN argList RPAREN
                 {
                    symEntry* declaration = ST_lookup($1);
                    if (declaration == NULL) {
                      yyerror("Undefined function");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (declaration->symbol_type != FUNCTION) {
                      yyerror("Calling non-function identifier as a function.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    checkArgList($3, $1, ARGLIST);
                    tree* idNode = maketreeWithVal(IDENTIFIER, declaration->index);
                    idNode->scope = strcmp(declaration->scope,"") == 0 ? 0 : 1;
                    idNode->stentry = declaration;
                    $$ = maketree(FUNCCALLEXPR);
                    $$->sym_type = declaration->data_type;
                    addChild($$, idNode);
                    addChild($$, $3);
                 }
                | ID LPAREN RPAREN
                 {
                    symEntry* declaration = ST_lookup($1);
                    if (declaration == NULL) {
                      yyerror("Undefined function");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (declaration->symbol_type != FUNCTION) {
                      yyerror("Calling non-function identifier as a function.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    if (declaration->size > 0) {
                      yyerror("Too few arguments provided in function call.");
                      printf("Compilation was aborted.\n");
                      YYABORT;
                    }
                    tree* idNode = maketreeWithVal(IDENTIFIER, declaration->index);
                    idNode->stentry = declaration;
                    idNode->scope = strcmp(declaration->scope,"") == 0 ? 0 : 1;
                    $$ = maketree(FUNCCALLEXPR);
                    $$->sym_type = declaration->data_type;
                    addChild($$, idNode);
                 }
                ;
                
argList         : expression
                 {
                    $$ = maketree(ARGLIST);
                    addChild($$, $1);
                 }
                | argList COMMA expression
                 {
                    $$ = $1;
                    addChild($$, $3);
                 }
                ;
                

%%

int yywarning(char * msg){
    printf("warning: line %d: %s\n", yylineno, msg);
    return 0;
}

int yyerror(char * msg){
    printf("error: line %d: %s\n", yylineno, msg);
    return 0;
}
