#include <stdint.h>
#include <string.h>
#include "combined_utils.h"
#include "assemble_utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "combined_utils.h"
#include "assemble_utils.h"

#define IS_ALPHA(x) (x >= 'a' && x<= 'z')
#define POS(x) (x - 'a')
#define ALPHABET_SIZE (26)

const char *DATA_PROCESSING[15] = {"and", "eor", "sub", "rsb", "add", "orr",
				   "mov", "tst", "teq", "cmp", "lsl"};
const char *MULTIPLY[5] = {"mul", "mla"};
const char *DATA_TRANSFER[5] = {"str", "ldr"};
const char *BRANCH[5] = {"b"};

// set from bit to given value? not sure if a good idea to refactor like that
//however it would be more versatile
static Instruction setBits(uint32_t value, int bit, Instruction instr) {
  return instr |= (value << bit);;
}

Dictionary *createDictionary(void){
  return createNode();
}

void freeDict(Dictionary *root){
  freeNode(root);
}

int find(Dictionary *root, const char *word) {
  if(!root){
    return 0;
  }
  if(!*word){
    return root -> mnemonicFlag;
  }
  if(!IS_ALPHA(*word)){
    return 0;
  }
  if(!root->children[POS(*word)]){
    return 0;
  }
  return find(root->children[POS(*word)], word + 1);
}
int insert(Dictionary *root, const char *word) {

  if(!*word){
    return root -> mnemonicFlag = 1;
  }
  if(!IS_ALPHA(*word)){
    return 0;
  }
  if(!root->children[POS(*word)]){
    root->children[POS(*word)] = createNode();
  }
  return insert(root->children[POS(*word)], word + 1);

}

Dictionary *create_node(void) {

  Dictionary *node = calloc(1, sizeof *node);
  if(node == NULL){
    perror("Node allocation failed");
    exit(EXIT_FAILURE);
  }

  node -> children = calloc(ALPHABET_SIZE, sizeof (*node->children));
  if(!node-> children){
    perror("Children array allocation failed");
    exit(EXIT_FAILURE);
  }
  node -> mnemonicFlag = 0;

  return node;
}

void free_node(Dictionary *root) {

  if(!root){
    return;
  }

  for(int i = 0; i < ALPHABET_SIZE; i++){
    free_node(root->children[i]);
  }

  free(root -> children);
  free(root);
}

// sets bits for instructions that
// compute results: and, eor, sub, rsb, add, orr
void withResults(Instruction instruction, char **code){
	int rd = atoi(strtok(code[1], "r"));
	int rn = atoi(strtok(code[2], "r"));
	int operand;
	if(code[3][0] == '#'){
		// set the I flag
		instruction = instruction | (1 << 25);

		// NEED TO ROTATE //
		if(code[3][2] == 'x'){
			// hexadecimal value
			operand = atoi(strtok(code[3], "#0x"));
		}
		else{
			// decimal value
			operand = atoi(strtok(code[3], "#"));
		}
		int rotations = 0;
		while((operand >> 8) != 0){
			rotations++;
			operand = (operand >> 1) | (operand << 32);
		}
		instruction = instruction | ((rotations/2) << 8);
		instruction = instruction | operand;
	}
	else{

	}


	instruction = instruction | (rd << 12);
	instruction = instruction | (rn << 16);
	instruction = instruction | operand;
}

// sets bits for instructions that
// do single operand assignment: mov
void operandAssignment(Instruction instruction, char **code){
	int rd = atoi(strtok(code[1], "r"));
	int operand = atoi(code[2]);

	instruction = instruction | (rd << 12);
	instruction = instruction | operand;
}

// sets bits for instructions that
// do not compute results but
// set the CPSR flags: tst, teq, cmp
void noResults(Instruction instruction, char **code){
	int rn = atoi(strtok(code[1], "r"));
	int operand = atoi(code[2]);

	instruction = instruction | (rn << 16);
	instruction = instruction | operand;

	instruction = instruction | (1 < 20);
}

// all of these probably take char *nextInstruction or nothing at all
static Instruction setDataProcessing(char **code) {

  Instruction instruction = 0;

  //sets condition code
  instruction = setBits(al, 28, instruction);
 
  // setting the opcode bits
  //if instruction is supposed to be tst, teq or cmp - set S - bit 20
  // otherwise it is clear (cleared upon initialisation)
  if(!strcmp(code[0], "and")){
		  withResults(instruction, code);
  }
  else if(!strcmp(code[0], "eor")){
	  withResults(instruction, code);
	  instruction = instruction | (1 << 21);
  }
  else if(!strcmp(code[0], "sub")){
	  withResults(instruction, code);
	  instruction = instruction | (2 << 21);
  }
  else if(!strcmp(code[0], "rsb")){
	  withResults(instruction, code);
	  instruction = instruction | (3 << 21);
  }
  else if(!strcmp(code[0], "add")){
	  withResults(instruction, code);
	  instruction = instruction | (4 << 21);
  }
  else if(!strcmp(code[0], "orr")){
	  withResults(instruction, code);
	  instruction = instruction | (12 << 21);
  }
  else if(!strcmp(code[0], "mov")){
	  operandAssignment(instruction, code);
	  instruction = instruction | (13 << 21);
  }
  else if(!strcmp(code[0], "tst")){
	  noResults(instruction, code);
	  instruction = instruction | (8 << 21); 
  }
  else if(!strcmp(code[0], "teq")){
	  noResults(instruction, code);
	  instruction = instruction | (9 << 21);
  }
  else if(!strcmp(code[0], "cmp")){
	  noResults(instruction, code);
	  instruction = instruction | (10 << 21);
  }

  return instruction;
}

void setRegistersMultiply(Instruction instruction, char **code){
	int rd = atoi(strtok(code[1], "r"));
	int rm = atoi(strtok(code[2], "r"));
	int rs = atoi(strtok(code[3], "r"));

	instruction = instruction | (rd << 16);
	instruction = instruction | rm;
	instruction = instruction | (rs << 8);
}

static Instruction setMultiply(char **code) {
  Instruction instruction = 0;

  // sets condition code
  instruction = setBits(al, 18, instruction);
  instruction = setBits(0x9, 4, instruction);


  // sets the register bits
  // and A bit if instructino is mla
  // there is no need to set S because instruction is initially all 0
  if(!strcmp(code[0], "mul")){
	  setRegistersMultiply(instruction, code);
  }
  else if(!strcmp(code[0], "mla")){
	  instruction = instruction | (1 << 21);
	  setRegistersMultiply(instruction, code);
	  int rn = atoi(strtok(code[4], "r"));
	  instruction = instruction | (rn << 12);
  }

  return instruction;
}

static Instruction setDataTransfer() {

  Instruction instruction = 0;
  
  // set condition code
  instruction = setBits(1, 26, instruction);
  


  return 0;
}

static Instruction setBranch(char **code) {

  Instruction  instruction = 0;

  // set the 101 at bits 27 - 25
  instruction = setBits(0x5, 25, instruction);


  // set condition code
  if(!strcmp(code[0], "bne")){
	  instruction = instruction | (1 << 28);
  }
  else if(!strcmp(code[0], "bge")){
	  instruction = instruction | (10 << 28);
  }
  else if(!strcmp(code[0], "blt")){
	  instruction = instruction | (11 << 28);
  }
  else if(!strcmp(code[0], "bgt")){
	  instruction = instruction | (12 << 28);
  }
  else if(!strcmp(code[0], "ble")){
	  instruction = instruction | (13 << 28);
  }
  else{
	  instruction = instruction | (14 << 28);
  }

  return instruction;
}

static Instruction setHalt() {
  return 0;
}

int contains(char *value, const char **array){
  for(int i = 0; i < sizeof(array)/sizeof(array[0]); i++){
    if(!strcmp(array[i], value))
      return 1;
  }
  return 0;
}

Instruction assemble(struct Dictionary *symbolTable, const char *nextInstruction) {
  /*
    char *opcode;
    char *rest = nextInstruction;


    opcode = strtok_r(rest, " ", &rest);
    if(contains(opcode, DATA_PROCESSING)){
    return setDataProcessing(nextInstruction);
    } else if(contains(opcode, MULTIPLY)){
    return setMultiply();
    } else if(contains(opcode, DATA_TRANSFER)){
    return setDataTransfer();
    } else{
    return setBranch();
    }
  */

  return 0;
}

char *getProgramError(errorCode e) {
  errorType errors[2];
  errors[INVALID_INSTRUCTION].code = INVALID_INSTRUCTION;
  errors[INVALID_INSTRUCTION].message = "Invalid instruction";
  //if we have more program error types add them above
  return errors[e].message;
}
