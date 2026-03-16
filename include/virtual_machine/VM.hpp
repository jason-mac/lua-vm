#pragma once
#include "Chunk.hpp"
#include "Object.hpp"
#include "Value.hpp"
#include <algorithm>
#include <unordered_map>

// InterpretResult
enum class InterpretResult
{
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR
};

struct CallFrame
{
  ObjectFunction* function;
  Byte* ip;
  Value* slots;
};

enum class Operator
{
  PLUS,
  MINUS,
  DIVIDE,
  MULTIPLY,
  MODULO,
  POWER,
  CONCAT,
  EQUAL,
  LESS_THAN,
  LESS_EQUAL,
};

class VM
{
public:
  static constexpr int UINT8_COUNT = sizeof(uint8_t) + 1;
  static constexpr int FRAMES_MAX = 64;
  static constexpr int STACK_MAX = FRAMES_MAX * UINT8_COUNT;
  CallFrame frames[FRAMES_MAX];
  int frameCount;
  Value stack[STACK_MAX];
  Value* stackTop;
  std::unordered_map<std::string, Value> globals;

  void init();
  void free();
  InterpretResult interpret(ObjectFunction*);
  void push(Value value);
  Value pop();
  InterpretResult run();
  void resetStack();
  Byte readByte(CallFrame*);
  int16_t readShort(CallFrame*);
  Value readConstant(CallFrame*);
  std::string readString(CallFrame*);
  void binaryOp(Operator oper);
  bool isFalsey(const Value& value);
  Value peek(int distance);
  bool callValue(Value callee, int argCount);
  bool call(ObjectFunction*, int);
  void defineNative(const std::string& name, NativeFn function);
};
