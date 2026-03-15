#pragma once
#include "Common.hpp"
// inspired from risc-v
enum class OpCode : Byte
{
  // arithmetic
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  POW,
  NEG, // unary minus
       // logic
  NOT, // unary not
  LEN, // # operator
       // comparison
  AND,
  OR,
  EQ,
  LT,
  LE, // ==, <, <=  (derive ~=, >, >= from these)
      // load
  LOAD_CONST,
  LOAD_NIL,
  LOAD_BOOL,
  // stack
  POP,
  MOVE,
  // control flow
  JMP,
  JMP_IF_FALSE,
  JMP_IF_TRUE,
  // variables
  GET_GLOBAL,
  SET_GLOBAL,
  GET_LOCAL,
  SET_LOCAL,
  // functions
  CALL,
  RETURN,
  CLOSURE,
  // string
  CONCAT, // ..
};
