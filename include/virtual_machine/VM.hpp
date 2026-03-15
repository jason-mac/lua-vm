#pragma once
#include "Chunk.hpp"
#include "Value.hpp"

// InterpretResult
enum class InterpretResult
{
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR
};

enum class Operator
{
  PLUS,
  MINUS,
  DIVIDE,
  MULTIPLY
};

class VM
{
public:
  static constexpr int STACK_MAX = 256;
  Chunk* chunk;
  size_t ip = 0;
  Value stack[STACK_MAX];
  Value* stackTop;

  void init();
  void free();
  InterpretResult interpret(Chunk* chunk);
  void push(Value value);
  Value pop();

private:
  InterpretResult run();
  void resetStack();
  Byte readByte();
  Value readConstant();
  void binaryOp(Operator oper);
};
