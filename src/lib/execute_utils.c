#include <stdint.h>
#include <stdlib.h>
#include "execute_utils.h"
#include "pipeline_utils.h"
#include <stdio.h>

#define CREATE_MASK(start, end) ((1 << (start + 1)) - (1 << end))
//creates a mask of 1s from start to end
#define OPERATOR_FUNCTION(name, operator)	\
  static Register name(const Register operand1, const Register operand2)\
  {\
    return operand1 operator operand2;\
  }
#define OPERATOR_NONSHIFT(name, operator)\
static Register name(const Register operand1, const Register operand2)\
  {\
    return operand1 operator operand2;\
  }
#define OPERATOR_SHIFT(name, operator)\
static Register name(const Register value, const uint32_t shift)\
  {\
    return value operator shift;\
  }


typedef int (*execution_function)(Instruction, State*);
typedef Register (*shift_function)(Register, uint32_t);

// defining operator functions for use in execute_data_processing
OPERATOR_FUNCTION(and, &)
OPERATOR_FUNCTION(eor, ^)
OPERATOR_FUNCTION(sub, -)
OPERATOR_FUNCTION(add, +)
OPERATOR_FUNCTION(or, |)
OPERATOR_FUNCTION(mov, *0+)

  static Register rsb(Register operand1, Register operand2) {
  return sub(operand2, operand1);
}

OPERATOR_FUNCTION(lsl, <<)
OPERATOR_FUNCTION(lsr, >>)

static uint32_t asr(uint32_t value, uint32_t shift){
  if (shift > 32) {
    shift = 32;
  }

  int sign = value & (1 << 31);
  value = value >> shift;

  if (sign) {
    return value | ~((1 << (32 - shift)) - 1); // CREATE_MASK(31, (32 - shift));
  }

  return value;
}

static uint32_t ror(uint32_t value, uint32_t shift){
  uint32_t shifted, rotated;
  shift %= 32;

  shifted = value >> shift;
  rotated = value << (32 - shift);
  return shifted | rotated;
}

// start of execute functions
int execute_halt(Instruction intruction, State *state){
  return 4;
}

// if you want to change the carry bit then carry = 1
// loads register offset given an instruction and state by checking bits 11-0
Register get_offset_register(int carry, Instruction instruction, State *state){
  Register value = *getRegPointer(3, state, instruction);

  // check bits 6-5 to find what shift to use on register value
  shift_function shifts[4] = {lsl, lsr, asr, ror};
  shift_function selectedShift = shifts[(instruction & CREATE_MASK(6, 5)) >> 5];

  uint32_t amount;
  
  // Check 4th bit
  if (instruction & (1 << 4)) {
    // 4th bit is 1
    amount = *getRegPointer(11, state, instruction);
  } else {
    // 4th bit is 0
    amount = (instruction & CREATE_MASK(11, 7)) >> 7;
  }

  if (carry) {
    int carryBit;
    
    if (selectedShift == lsl) {
      carryBit = (instruction & (1 << (32 - amount))) >> (32 - amount);
    } else {
      carryBit = (instruction & (1 << (amount - 1))) >> (amount - 1);
    }

    setC(state, carryBit);
  }

  return selectedShift(value, amount);
}

static int execute_data_processing(Instruction instruction, State *state) {
  Register operand1, operand2, destination;
  Operand operand = {0};
  
  //operand1 = *(state + (instruction & CREATE_MASK(19,16)));
  //destination = *(state + (instruction & CREATE_MASK(15,12)));
  //TODO fix these lines
  switch(instruction & CREATE_MASK(24,21)) {
    case 0: 
      operand.operation = and;
      operand.isWritten = 1;
      break;
    case 1:
      operand.operation = eor;
      operand.isWritten = 1;
      break;
    case 2:
      operand.operation = sub;
      operand.isWritten = 1;
      operand.isArithmetic = 1;
      break;
    case 3:
      operand.operation = rsb;
      operand.isWritten = 1;
      operand.isArithmetic = 1;
      break;
    case 4:
      operand.operation = add;
      operand.isWritten = 1;
      operand.isArithmetic = 1;
      break;
    case 8:
      operand.operation = and;
      break;
    case 9:
      operand.operation = eor;
      break;
    case 10:  
      operand.operation = sub;
      operand.isArithmetic = 1;
      break;
    case 12:
      operand.operation = or;
      operand.isWritten = 1;
      break;
    case 13:
      operand.operation = mov;
      operand.isWritten = 1;
      break;
    default:
      perror("ERROR: invalid operand");
      return 1;
  }
  
  if (instruction & (1 << 25)) {
    // operand2 is an immediate constant
    operand2 = instruction & CREATE_MASK(7, 0);
    // rotate  = rotate * 2
    uint32_t rotate = (instruction & CREATE_MASK(11, 8)) >> 7;

    operand2 = ror(operand2, rotate);
  }
  
  return 0;
}


Register *getRegPointer(int reg, State *currentState, Instruction instruction){
	Register *regPtr = &currentState->reg0;
	// getting the 4 bits of the instruction
	// which correspond to the register which
	// starts at bit reg
	int regAddress = instruction & ((1 << (reg + 1)) - 1) >> (reg - 4);
	regPtr += regAddress;
	return regPtr;
}


int execute_multiply(Instruction instruction, State *state){
      //get destination register pointer - bits 19 to 16 in instr
      // Rm - bits 3 to 0
      // Rs - bits 11 to 8
      
      Register *regRd = getRegPointer(19, state, instruction);
      Register *regRs = getRegPointer(11, state, instruction);
      Register *regRm = getRegPointer(3, state, instruction);

      
      // check whether to perform multiply and accumulate or
      // multiply only - function for now in pipeline_utils
      if(getA(instruction)){

	// set destination register to Rm x Rs + Rn
	// Rn - bits 15 to 12

	Register *regRn = getRegPointer(15, state, instruction);

	*regRd = (*regRm) * (*regRs) + (*regRn);	

      } else{

	//set destination register to Rm x Rs
        *regRd = (*regRm) * (*regRs);
	
      }

      if(setCPSR(instruction)){

	//set result to the value in destination register
	int result = *regRd;
	setZ(state, result);
	setN(state, result);

      }

      // return 1 if successful;
      return 1;
}

int execute_data_transfer(Instruction instruction, State *state) {

  int offset = instruction & ((1 << 12) - 1);
	// here the function which calculates the offset
	// depending on the I bit of the instruction
  
	Register *destReg = getRegPointer(15, state, instruction);
	Register *baseReg = getRegPointer(19, state, instruction);
	
	int memAddress = 0;

	if(!getU(instruction)){
		// subtracting offset
		offset = -offset;
	}
	if(getI(instruction)){
		offset = get_offset_register(0, instruction, state);
	}	
	if(getP(instruction)){
		// pre-indexing
		if(getL(instruction)){
			// loading
			*destReg = state->memory[*baseReg + offset];
		} else {
			// storing
			memAddress = *baseReg + offset;
			state->memory[memAddress] = CREATE_MASK(7, 0) & *destReg;
			state->memory[memAddress + 1] = CREATE_MASK(7, 0) & (*destReg >> 8);
			state->memory[memAddress + 2] = CREATE_MASK(7, 0) & (*destReg >> 16);
			state->memory[memAddress + 3] = CREATE_MASK(7, 0) & (*destReg >> 24);
		}
	} else {
		// post-indexing
		if(getL(instruction)){
			// loading
			*destReg = state->memory[*baseReg];
			*baseReg += offset;
		} else {
			// storing
			state->memory[*baseReg] = *destReg;
			*baseReg += offset;
		}
	}
	return 1;
}

int execute_branch(Instruction instruction, State *state){
	int32_t extendedOffset = (CREATE_MASK(23, 0) & instruction) << 2;
	state->regPC += extendedOffset;
       	return 1;
}

int execute(Instruction instruction, State *state, InstructionType type) {
  //instruction to be executed, machine state, boolean on whether to execute, instruction type
  execution_function executions[5];

  //initialises the array with function pointers for each instruction type
  //based on the ordering of instr_type enum
  executions[0] = execute_branch;
  executions[1] = execute_data_transfer;
  executions[2] = execute_data_processing;
  executions[3] = execute_multiply;
  executions[4] = execute_halt;
  
  return (executions[type](instruction, state));
  //executes the function corresponding to the type of instruction by the enum's value
}
