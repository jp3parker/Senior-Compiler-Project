#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "strtab.h"
#include <stdbool.h>

param *working_list_head = NULL;
param *working_list_end = NULL;
table_node *current_scope = NULL;
int working_list_size = 0;
extern int yyerror();

/* Provided is a hash function that you may call to get an integer back. */
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

symEntry* ST_insert(char *id, int data_type, int symbol_type, char* scope, int size){
    if (DEBUG) printf("ST_insert(%s,%i,%i,%s) called...\n",id,data_type,symbol_type,scope);
    unsigned long int key = hash((unsigned char*)id) % MAXIDS;
    bool empty = current_scope->strTable[key] == NULL;
    bool found = !empty && strcmp(current_scope->strTable[key]->id, id) == 0;
    while (!empty && !found) {
      if(++key == MAXIDS) {
          key = 0;
      }
      empty = current_scope->strTable[key] == NULL;
      found = !empty && strcmp(current_scope->strTable[key]->id, id) == 0;
    }
    if (!found) { //found empty spot before id in all scopes
      current_scope->strTable[key] = calloc(1, sizeof(symEntry));
      current_scope->strTable[key]->id = id;
      if (DEBUG) printf("the id entered = %s\n", current_scope->strTable[key]->id);
      current_scope->strTable[key]->scope = scope;
      current_scope->strTable[key]->data_type = data_type;
      current_scope->strTable[key]->symbol_type = symbol_type;
      current_scope->strTable[key]->size = size;
      current_scope->strTable[key]->index = key;
    }
    else { //found id before empty spot in search->scope
	    yyerror("Symbol declared multiple times.");
      printf("Compilation was aborted.\n");
      exit(-1);
    }
    
    if (DEBUG) printf("...ST_insert(%s,%i,%i,%s) finished\n",id,data_type,symbol_type,scope);
    return current_scope->strTable[key];
}

symEntry* ST_lookup(char *id){
    if (DEBUG) printf("ST_lookup(%s) called...\n", id);

    unsigned long int original_key = hash((unsigned char*)id) % MAXIDS;
    unsigned long int key = original_key;
    table_node* search_scope = current_scope;
    bool found = false;
    while (search_scope != NULL && found == false) {
      key = original_key;
      while(search_scope->strTable[key] != NULL &&
      strcmp(search_scope->strTable[key]->id, id) != 0) {
        if(++key == MAXIDS) {
          key = 0;
        }
      }
      if (search_scope->strTable[key] != NULL) {
        found = true;
      }
      else {
        search_scope = search_scope->parent;
      }
    }
    if (DEBUG) printf("...ST_lookup(%s) finished\n", id);
    if (!found) { //found empty spot before id in all scopes
      return NULL;
    }
    else { //found id before empty spot in search->scope
	    return search_scope->strTable[key];
    }

}

void new_scope(){
    if (DEBUG) printf("new_scope() called...\n");

    table_node* new_scope = calloc(1, sizeof(table_node));
    new_scope->numChildren = 0;
    new_scope->first_child = NULL;
    new_scope->last_child = NULL;
    new_scope->next = NULL;
    new_scope->parent = NULL;

    if(current_scope == NULL){
      current_scope = new_scope;
    }
    else {
      new_scope->parent = current_scope;
      if (current_scope->numChildren == 0) {
        current_scope->first_child = new_scope;
        current_scope->last_child = new_scope;
      }
      else {
        current_scope->last_child->next = new_scope;
        current_scope->last_child = new_scope;
      }
      ++current_scope->numChildren;
      current_scope = new_scope;
    }
    
    if (DEBUG) printf("...new_scope() finished\n");
}

void up_scope(){
    if (DEBUG) printf("up_scope() called...\n");

    if(current_scope->parent != NULL){
      current_scope = current_scope->parent;
    } else {
      if (DEBUG) printf("...up_scope() finished\n");
      return;
    }
    
    if (DEBUG) printf("...up_scope() finished\n");
}

//void output_entry(int i){ //commented out to make compile
//    printf("%d: %s ", i, types[strTable[i].data_type]);
//    printf("%s:%s%s\n", strTable[i].scope, strTable[i].id, symTypeMod[strTable[i].symbol_type]);
//}


// assuming an index is passed that is of valid range
char* get_symbol_id(int index, int scope) {
  table_node* node = current_scope;
  if (scope == 0 && node->parent != NULL) {
    node = node->parent;
  }
  return node->strTable[index]->id;
}

void setSize(int index, int size) {
  current_scope->strTable[index]->size = size;
}

void addParameter(int index, int data_type, int symbol_type) {
  param* currentParam = current_scope->parent->strTable[index]->params;
  if (currentParam == NULL) { // first parameter in function
    current_scope->parent->strTable[index]->params = malloc(sizeof(param));
    current_scope->parent->strTable[index]->params->next = NULL;
    current_scope->parent->strTable[index]->params->data_type = data_type;
    current_scope->parent->strTable[index]->params->symbol_type = symbol_type;
  }
  else {
    while (currentParam->next != NULL) {
      currentParam = currentParam->next;
    }
    currentParam->next = malloc(sizeof(param));
    currentParam = currentParam->next;
    currentParam->next = NULL;
    currentParam->data_type = data_type;
    currentParam->symbol_type = symbol_type;
  }
  
  if (DEBUG) {
    printf("printing parameter list for %s:\n", current_scope->parent->strTable[index]->id);
    param* current = current_scope->parent->strTable[index]->params;
    do {
      printf("data_type: %d   symbol_type: %d\n", current->data_type, current->symbol_type);
      current = current->next;
    } while (current);
  }
  
}

void addToWorkingList(int data_type, int symbol_type) {
  param* newParam = malloc(sizeof(param));
  newParam->data_type = data_type;
  newParam->symbol_type = symbol_type;
  newParam->next = NULL;
  if (working_list_head == NULL) {
    working_list_head = newParam;
    working_list_end = newParam;
  }
  else {
    working_list_end->next = newParam;
    working_list_end = newParam;
  }
}

void emptyWorkingList() {
  while (working_list_head) {
    param* temp = working_list_head;
    working_list_head = working_list_head->next;
    free(temp);
  }
}

void print_sym_tab(char* name) {
  printf("%s SYMBOL TABLE:\n", name);
  table_node* c = current_scope;
  for (int i=0; i<MAXIDS; ++i) {
    if (c->strTable[i] != NULL) {
      printf("#%d: ID=\"%s\"\t\tscope=\"%s\"", i, c->strTable[i]->id, \
      c->strTable[i]->scope ? c->strTable[i]->scope : "");
      if (c->strTable[i]->symbol_type == FUNCTION) {
        printf("\n\treturn-> %s", c->strTable[i]->data_type == INT_TYPE ? "int" : c->strTable[i]->data_type == CHAR_TYPE ? "char" : "void");
        printf("\tparams(");
        param* cur_param = c->strTable[i]->params;
        while (cur_param != NULL) {
          if (cur_param->data_type == INT_TYPE) {
            printf("int");
          }
          else if (cur_param->data_type == CHAR_TYPE) {
            printf("char");
          }
          else {
            printf("void");
          }
          if (cur_param->symbol_type == ARRAY) {
            printf("[]");
          }
          if (cur_param->next != NULL) {
            printf(", ");
          }
          cur_param = cur_param->next;
        }
        printf(")\n");
      }
      else if (c->strTable[i]->symbol_type == ARRAY) {
        printf("\tarray\n");
      }
      else {
        printf("\tscalar\n");
      }
    }
  }
}

void printGlobalVars(FILE* outfile) {
  if (current_scope->parent != NULL) current_scope = current_scope->parent;
  
  for (int i=0; i<MAXIDS; ++i) {
    if (current_scope->strTable[i] != NULL &&
        current_scope->strTable[i]->data_type == INT_TYPE &&
        current_scope->strTable[i]->symbol_type == SCALAR) {
      fprintf(outfile, "st%d:\t.word 0\n", i);
    }
  }
}
