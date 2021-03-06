#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "assemble_utils.h"
#include "text_utils.h"

/* Creates a mask of 1s from start to end */
#define CREATE_MASK(start, end) ((1 << (start + 1)) - (1 << end))

/* Takes an instruction and leaves it in the form:
        char** = {label, arg1, arg2 ...}
   Pre: line is in correct format,
        destArray is NULL terminated */
static int instructionTok(char **destArray, char *line) {
  assert(strlen(line) <= MAX_INSTRUCTION_SIZE);

  if(!destArray || !line) {
    return INVALID_INSTRUCTION;
  }

  for (int i = 0; i < MAX_INSTRUCTION_PARAMS + 1; i++) {
    if (destArray[i]) {
      free(destArray[i]);
      destArray[i] = NULL;
    }
  }
  
  char *rest = line;
  char *check = strtok_r(line, " \n", &rest);
  
  if (!check) {
    destArray[0] = NULL;
    return WHITESPACE_LINE;
  }
  
  destArray[0] = strdup(check);
    
  int i = 0;
  int j, k;
    
  while(check) {
    i++;
    
    check = strtok_r(rest, " ,\n", &rest);
    if (!check) {
      destArray[i] = NULL;
      break;
    }
    
    destArray[i] = strdup(check);
    k = 0;
    while(rest[k] == ' ') {
      k++;
    }
    rest = &rest[k];
    if(rest[0] == '[') {
      i++;
      j = 1;
      while(rest[j] != ']') {
        j++;
      }
      
      destArray[i] = strdup(rest);
      destArray[i][j + 1] = '\0';
      rest = &rest[j + 2];
    }
  }
  return OK;
}

int loadNextInstruction(char **destArray, FILE *sourceFile) {
  if(!sourceFile) {
      printf("Could not access file");
      return NULL_FILE;
  }
  char line[MAX_INSTRUCTION_SIZE];
  if (!fgets(line, MAX_INSTRUCTION_SIZE, sourceFile)) {
    if(feof(sourceFile)) {
        return END_OF_FILE;
    }
    return SYS; //feof failed because of a system fault
  }

  return instructionTok(destArray, line);
}

int writeNextInstruction(Instruction next, FILE *outputFile) {
  Instruction instruction = (CREATE_MASK(7, 0) & next)
	| (CREATE_MASK(15, 8) & next)
	| (CREATE_MASK(23, 16) & next)
	| (~((1 << 24) - 1)  & next);
  if(!outputFile) {
    printf("Could not access file");
    return NULL_FILE;
  }
  fwrite(&instruction, 4, 1, outputFile);
  return OK;
}

int isLabel(char **line) {
  /* Take the first token of line */
  char *currentChar = *line;

  if(IS_ALPHA(*currentChar)) {
    /* First character of a label must be alphabetical */
    while(*(currentChar + 1) != '\0') {
      currentChar++;
      if(IS_ALPHANUMERIC(*currentChar)) {
	continue; 
      } else if(*currentChar == ':') {
	if(*(currentChar + 1) == '\0') {
	  /* First parameter is alphanumeric, terminated with a ':' */
	  return 1; 
	} else {
	  /* Not a valid label as colon in middle of string */
	  return 0;
	}
      } else {
	/* Not a valid label as there are invalid characters for a label */
	return 0; 
      }
    }
  } else {
    return -1;
  }
  /* No ending ':' found on first parameter so no label */
  return 0;
}
