#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "combinedlib/combined_utils.h"
#include "assemblelib/assemble_utils.h"
#include "assemblelib/text_utils.h"

assemblyInstruction getDataFromOperation(char *operation) {
  assemblyInstruction out = {0};
  return out;
}

Instruction *makeExtraInstructionsArray(Address size) {
  Instruction *out = calloc(size, sizeof(Instruction));
  return out;
}

/*
  argv[1] -> input file
  argv[2] -> output file
*/
/* Implementation of a two-pass assembler */
int main(int argc, char **argv) {
  assert (argc == 3);

  errorCode currentStatus = OK;
  char *source = argv[1], *dest = argv[2];
  FILE *sourceFile, *destFile;
  symbolNode *symbolTable = NULL; //creates an empty symbol table
  char **currentLine = calloc((MAX_INSTRUCTION_PARAMS + 1), sizeof(char*));
  FATAL_SYS((currentLine == NULL));
  for(int i = 0; i < (MAX_INSTRUCTION_PARAMS + 1); i++){
    currentLine[i] = calloc((MAX_INSTRUCTION_SIZE + 1), sizeof(char));
    FATAL_SYS((currentLine[i] == NULL));
  }
  ldrAddresses loadConstants = {0};
  Address currAddress = 0;

  /* First pass -> creating the symbol table */
  sourceFile = fopen(source, "r");
  FATAL_SYS((sourceFile == NULL)); //file loading failed
  currentStatus = loadNextInstruction(currentLine, sourceFile);
  FATAL_PROG((currentStatus != OK && currentStatus != END_OF_FILE), currentStatus);
  int label;
  treeData data;
  while(currentStatus != END_OF_FILE) {
    /* Check if the first token is a label */
    label = isLabel(currentLine);
    if(label == 1) {
      data.address = currAddress; 
    } else if(label == -1) {
      /* An invalid instruction is detected */
      FATAL_PROG(1,INVALID_INSTRUCTION);
    } else {
      data.assemblyLine = getDataFromOperation(currentLine[0]);
    }
    //adds to the symbol table
    symbolTable = insert(symbolTable, currentLine[0], data, label);
    /* Load next instruction and increment address by 4 */
    currentStatus = loadNextInstruction(currentLine, sourceFile);
    FATAL_PROG((currentStatus != OK && currentStatus != END_OF_FILE), currentStatus);
    currAddress = currAddress + 4;
  }

  currentStatus = OK;
  FATAL_SYS((fclose(sourceFile) != 0));

  /* Second pass -> generate encoded instructions */
  loadConstants.lastAddress = currAddress; //largest address

  /*max number of elements is number of instructions assembled 
    if every instruction is an ldr with immediate value */

  Instruction *extraInstructions = makeExtraInstructionsArray(currAddress);
  FATAL_PROG(extraInstructions == NULL, OUT_OF_MEMORY);
  
  loadConstants.extraInstructions = extraInstructions;

  currAddress = 0;
  /* Reopen source file to go to the top */
  sourceFile = fopen(source, "r");
  FATAL_SYS((sourceFile == NULL));
  destFile = fopen(dest, "w");
  FATAL_SYS((destFile == NULL));

  currentStatus = loadNextInstruction(currentLine, sourceFile);
  FATAL_PROG((currentStatus != OK && currentStatus != END_OF_FILE), currentStatus);
  Instruction result = 0;
  
  while(currentStatus != END_OF_FILE) {
    currentStatus = assemble(&result, symbolTable, currentLine);

    //detects if something goes wrong assembling an instruction
    FATAL_PROG(currentStatus != OK, currentStatus);
    
    writeNextInstruction(result, destFile);
    currentStatus = loadNextInstruction(currentLine, sourceFile);
    FATAL_PROG((currentStatus != OK && currentStatus != END_OF_FILE), currentStatus);
    currAddress = currAddress + 4;
  }

  //Adds offsets for immediate value ldrs to the end of the assembly code
  for (int i = 0; i < loadConstants.length; i++) {
    writeNextInstruction(loadConstants.extraInstructions[i], destFile);
  }
  
  FATAL_SYS((fclose(sourceFile) != 0));
  FATAL_SYS((fclose(destFile) != 0));
  return EXIT_SUCCESS;

 fatalError:
  /* Free any dynamically alocated memory */

  if (currentLine != NULL) {
    for(int i = 0; i < (MAX_INSTRUCTION_PARAMS + 1); i++){
      free(currentLine[i]);
    }
  }
  free(currentLine);
  freeTable(symbolTable);
  /* Print an error message*/

  char *errorMessage;
  if (EC_IS_SYS_ERROR(currentStatus)) {
    errorMessage = strerror(EC_TO_SYS_ERROR(currentStatus));
  } else {
    errorMessage = getProgramError(currentStatus);
  }

  printf("%s\n", errorMessage);
  return EXIT_FAILURE;
}
