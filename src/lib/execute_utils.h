#include "pipeline_utils.h"
#ifndef EXECUTE_UTILS
#define EXECUTE_UTILS

typedef struct Operator {
  int isArithmetic;
  int isWritten;
  union {
    Register (*operationWithoutCarry) (Register, Register);
    Register (*operationWithCarry) (int, State *, Register, Register);
  } operation;
} Operator;

// return a pointer to a register whose address
// is the 4 bits of instruction starting from bit reg
Register *getRegPointer(int reg, State *currentState, Instruction instruction);

int execute(Instruction instruction, State *state, InstructionType type);

#endif
