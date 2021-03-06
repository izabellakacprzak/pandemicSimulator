#ifndef TREE
#define TREE

#include <stdint.h>
#include "../combinedlib/combined_utils.h"

/* Struct containing the details of a mnemonic */
typedef struct assemblyInstruction {
  ConditionCode conditionCode;
  InstructionType type;
  opcode code;
} assemblyInstruction;

/* Union used to store the data for a mnemonic or the adress of a label */
typedef union treeData {
  Address address;
  assemblyInstruction *assemblyLine;
} treeData;

/* A binary tree structure used for the symbol table */
typedef struct symNodeStruct {
  char* symbol;
  struct symNodeStruct* left;
  struct symNodeStruct* right;
  treeData data;
  int isLabel;
} symbolNode;

/* Inserts a new node into the tree,
   return NULL if node is already inserted */
symbolNode* insert(symbolNode * root, char* sym, treeData data, int isLabel);

/* Search for an item in the tree on the basis of sym,
   return the node containing sym */
symbolNode* search(symbolNode * root, char* sym);

/* Frees all dynamically allocated nodes */
void freeTable(symbolNode *root);

/* Allocates a new node and adds the symbol and address to it */
symbolNode *createNode(const char* sym, treeData data, int isLabel);

/* Gets the correct data from the instruction 
   mnemonic which is later added to the treeData */ 
assemblyInstruction *getDataFromOperation(char *operation);

#endif
