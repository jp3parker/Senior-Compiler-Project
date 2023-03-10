#ifndef STRTAB_H
#define STRTAB_H
#define MAXIDS 1000

#include <stdlib.h> // allows use of "NULL"
#include <stdio.h>
#define DEBUG 0

extern int yywarning(char*); //to silence implicit definition warning
enum dataType {INT_TYPE, CHAR_TYPE, VOID_TYPE, INT_ARRAY_TYPE, CHAR_ARRAY_TYPE, VOID_ARRAY_TYPE};
enum symbolType {SCALAR, ARRAY, FUNCTION};

typedef struct param{
    int data_type;
    int symbol_type;
    struct param* next;
} param;

typedef struct strEntry{
    int stackoffset;
    char* id;
    char* scope;
    int   data_type;
    int   symbol_type;
    int   size; //Num elements if array, num params if function
    int index; //index number of symEntry
    param*  params;
} symEntry;

/* You should use a linear linklist to keep track of all parameters passed to a function. The working_list_head should point to the beginning of the linklist and working_list_end should point to the end. Whenever a parameter is passed to a function, that node should also be added in this list. */
extern int working_list_size;
extern param *working_list_head;
extern param *working_list_end;

typedef struct table_node{
    symEntry* strTable[MAXIDS];
    int numChildren;
    struct table_node* parent;
    struct table_node* first_child; // First subscope
    struct table_node* last_child;  // Most recently added subscope
    struct table_node* next; // Next subscope that shares the same parent
} table_node; // Describes each node in the symbol table tree and is used to implement a tree for the nested scope as discussed in lecture 13 and 14.

extern table_node* current_scope; // A global variable that should point to the symbol table node in the scope tree as discussed in lecture 13 and 14.



/* Inserts a symbol into the current symbol table tree. Please note that this function is used to insert into the tree of symbol tables and NOT the AST. Start at the returned hash and probe until we find an empty slot or the id.  */
symEntry* ST_insert(char *id, int data_type, int symbol_type, char* scope, int size);
// int ST_insert(char *id, char *scope, int data_type, int symbol_type);

/* The function for looking up if a symbol exists in the current_scope. Always start looking for the symbol from the node that is being pointed to by the current_scope variable*/
symEntry* ST_lookup(char *id);
//int ST_lookup(char *id, char *scope);

// Creates a new scope within the current scope and sets that as the current scope.
void new_scope();

// Moves towards the root of the sym table tree.
void up_scope();

char* get_symbol_id(int index, int scope);

void setSize(int index, int size);

void addParameter(int index, int data_type, int symbol_type);

void print_sym_tab(char* name);

void printGlobalVars(FILE* outfile);

#endif //STRTAB_H
