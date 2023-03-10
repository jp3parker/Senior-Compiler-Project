#ifndef TREE_H
#define TREE_H

#define MAXCHILDREN 100

#include<strtab.h>
#include <stdbool.h>

extern int yyerror();

typedef struct treenode tree;

/* tree node - you may want to add more fields */
struct treenode {
      struct strEntry* stentry;
      int nodeKind;
      int numChildren;
      int val;
      int scope; // Used for var/id. Index of the scope. This works b/c only global and local.
      int type;
      int sym_type; // will be 0,1,2,3,4,5 for int,char,void,int[],char[],void[]
      tree *parent;
      tree *children[MAXCHILDREN];
};

tree *ast; /* pointer to AST root */

/* builds sub tree with zero children  */
tree *maketree(int kind);

/* builds sub tree with leaf node */
tree *maketreeWithVal(int kind, int val);

void addChild(tree *parent, tree *child);

void printAst(tree *root, int nestLevel);

/* Adds all children of sublist to list */
void flattenList(tree *list, tree *subList);

void checkArgList(tree* argList, char* id, int arglListVal);

param* getType(tree* node);

void checkExpression(tree* expression, tree* var);

/* tree manipulation macros */
/* if you are writing your compiler in C, you would want to have a large collection of these */

#define nextAvailChild(node) node->children[node->numChildren]
#define getChild(node, index) node->children[index]

#endif //TREE_H
