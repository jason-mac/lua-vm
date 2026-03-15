#include "virtual_machine/VM.hpp"
#include "Common.hpp"
#include "Debug.hpp"
#include "virtual_machine/Chunk.hpp"
#include "virtual_machine/OpCode.hpp"
#include <iostream>
#include <variant>

void VM::free() {}

void VM::resetStack()
{
  this->stackTop = this->stack;
}

void VM::push(Value value)
{
  *(this->stackTop) = value;
  this->stackTop++;
}

Value VM::pop()
{
  this->stackTop--;
  return *(this->stackTop);
}

InterpretResult VM::interpret(Chunk* chunk)
{
  this->chunk = chunk;
  return run();
}

void VM::init()
{
  resetStack();
}

Byte VM::readByte()
{
  return this->chunk->code[ip++];
}

Value VM::readConstant()
{
  return this->chunk->constants[this->readByte()];
}

static std::function<Value()> binaryFunc(Operator oper, Value left, Value right)
{
  double a = std::get<double>(left);
  double b = std::get<double>(right);
  switch (oper)
  {
    case Operator::PLUS: return [a, b]() { return Value(a + b); };
    case Operator::MINUS: return [a, b]() { return Value(a - b); };
    case Operator::DIVIDE: return [a, b]() { return Value(a / b); };
    case Operator::MULTIPLY: return [a, b]() { return Value(a * b); };
    default: throw std::runtime_error("Unknown operator");
  }
}

void VM::binaryOp(Operator oper)
{
  Value b = pop();
  Value a = pop();
  push(binaryFunc(oper, a, b)()); // push the result
}

InterpretResult VM::run()
{
  for (;;)
  {
    Byte instruction = this->chunk->code[ip++];
    switch (static_cast<OpCode>(instruction))
    {
      case OpCode::ADD: binaryOp(Operator::PLUS); break;
      case OpCode::SUB: binaryOp(Operator::MINUS); break;
      case OpCode::DIV: binaryOp(Operator::DIVIDE); break;
      case OpCode::MUL: binaryOp(Operator::MULTIPLY); break;
      case OpCode::RETURN:
      {
        printValue(pop());
        std::cout << '\n';
        return InterpretResult::OK;
      }
      case OpCode::NEG: push(-std::get<double>(pop())); break;
      case OpCode::LOAD_CONST:
      {
        Value constant = readConstant();
        printValue(constant);
        std::cout << '\n';
        break;
      }
      default: break;
    }
  }
}
