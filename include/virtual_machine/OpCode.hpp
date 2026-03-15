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

  AND,
  OR,

  // logic
  NOT, // unary not
  LEN, // # operator

  // comparison
  EQ,
  LT,
  LE, // ==, <, <=  (derive ~=, >, >= from these)

  // load
  LOAD_CONST,
  LOAD_NIL,
  LOAD_BOOL,
  MOVE,

  // control flow
  JMP,
  JMP_IF_FALSE,
  TEST,

  // variables
  GET_GLOBAL,
  SET_GLOBAL,

  // functions
  CALL,
  RETURN,
  CLOSURE,

  // string
  CONCAT, // ..
};
