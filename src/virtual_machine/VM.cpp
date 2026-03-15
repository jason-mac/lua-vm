#include "virtual_machine/VM.hpp"
#include "Common.hpp"
#include "Debug.hpp"
#include "Object.hpp"
#include "virtual_machine/Chunk.hpp"
#include "virtual_machine/OpCode.hpp"
#include <iostream>
#include <variant>

void VM::free() {}

void VM::resetStack()
{
  this->stackTop = this->stack;
  this->frameCount = 0;
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

InterpretResult VM::interpret(ObjectFunction* function)
{
  if (function == nullptr) return InterpretResult::COMPILE_ERROR;
  push(function);
  call(function, 0);
  return run();
}

void VM::init()
{
  resetStack();
  globals["print"] = newNative(
      [](int argc, Value* args) -> Value
      {
        for (int i = 0; i < argc; i++) printValue(args[i]);
        std::cout << "\n";
        return NilValue{};
      });
}

Byte VM::readByte(CallFrame* frame)
{
  return *frame->ip++;
}

uint16_t VM::readShort(CallFrame* frame)
{
  uint16_t high = frame->ip[0];
  uint16_t low = frame->ip[1];
  frame->ip += 2;
  return (int16_t)((high << 8) | low);
}

Value VM::readConstant(CallFrame* frame)
{
  return frame->function->chunk.constants[readByte(frame)];
}

std::string VM::readString(CallFrame* frame)
{
  return std::get<std::string>(readConstant(frame));
}

static std::function<Value()> binaryFunc(Operator oper, Value left, Value right)
{
  switch (oper)
  {
    case Operator::CONCAT:
    {
      std::string a = std::get<std::string>(left);
      std::string b = std::get<std::string>(right);
      return [a, b]() { return Value(a + b); };
    }
    case Operator::EQUAL: return [left, right]() { return Value(left == right); };
    case Operator::LESS_THAN:
    {
      double a = std::get<double>(left);
      double b = std::get<double>(right);
      return [a, b]() { return Value(a < b); };
    }
    case Operator::LESS_EQUAL:
    {
      double a = std::get<double>(left);
      double b = std::get<double>(right);
      return [a, b]() { return Value(a <= b); };
    }
    default:
    {
      double a = std::get<double>(left);
      double b = std::get<double>(right);
      switch (oper)
      {
        case Operator::PLUS: return [a, b]() { return Value(a + b); };
        case Operator::MINUS: return [a, b]() { return Value(a - b); };
        case Operator::DIVIDE: return [a, b]() { return Value(a / b); };
        case Operator::MULTIPLY: return [a, b]() { return Value(a * b); };
        case Operator::MODULO: return [a, b]() { return Value(std::fmod(a, b)); };
        case Operator::POWER: return [a, b]() { return Value(std::pow(a, b)); };
        default: throw std::runtime_error("Unknown operator");
      }
    }
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
  CallFrame* frame = &this->frames[this->frameCount - 1];
  for (;;)
  {
    Byte instruction = readByte(frame);
    switch (static_cast<OpCode>(instruction))
    {
      case OpCode::ADD: binaryOp(Operator::PLUS); break;
      case OpCode::SUB: binaryOp(Operator::MINUS); break;
      case OpCode::DIV: binaryOp(Operator::DIVIDE); break;
      case OpCode::MUL: binaryOp(Operator::MULTIPLY); break;
      case OpCode::MOD: binaryOp(Operator::MODULO); break;
      case OpCode::POW: binaryOp(Operator::POWER); break;
      case OpCode::NEG: push(-std::get<double>(pop())); break;
      case OpCode::NOT: push(!isFalsey(pop())); break;
      case OpCode::LEN: break;
      case OpCode::EQ: binaryOp(Operator::EQUAL); break;
      case OpCode::LT: binaryOp(Operator::LESS_THAN); break;
      case OpCode::LE: binaryOp(Operator::LESS_EQUAL); break;
      case OpCode::LOAD_CONST:
      {
        Value constant = readConstant(frame);
        push(constant);
        break;
      }
      case OpCode::LOAD_NIL: push(NilValue{}); break;
      case OpCode::LOAD_BOOL:
      {
        bool value = readByte(frame);
        push(value);
        break;
      }
      case OpCode::POP: pop(); break;
      case OpCode::JMP:
      {
        int16_t offset = readShort(frame);
        frame->ip += offset;
        break;
      }
      case OpCode::MOVE: break;
      case OpCode::AND:
      {
        Value b = pop();
        Value a = pop();
        push(isFalsey(a) ? a : b);
        break;
      }
      case OpCode::OR:
      {
        Value b = pop();
        Value a = pop();
        push(isFalsey(a) ? b : a);
        break;
      }
      case OpCode::JMP_IF_TRUE:
      {
        Value value = *(stackTop - 1);
        int16_t offset = readShort(frame);
        if (!isFalsey(value)) frame->ip += offset;
        break;
      }
      case OpCode::JMP_IF_FALSE:
      {
        Value value = *(stackTop - 1);
        int16_t offset = readShort(frame);
        if (isFalsey(value)) frame->ip += offset;
        break;
      }
      case OpCode::GET_GLOBAL:
      {
        std::string name = readString(frame);
        auto it = globals.find(name);
        if (it == globals.end()) push(NilValue{});
        else push(it->second);
        break;
      }
      case OpCode::SET_GLOBAL:
      {
        std::string name = readString(frame);
        globals[name] = *(stackTop - 1);
        break;
      }
      case OpCode::GET_LOCAL:
      {
        int8_t slot = readByte(frame);
        push(frame->slots[slot]);
        break;
      }
      case OpCode::SET_LOCAL:
      {
        Byte slot = readByte(frame);
        frame->slots[slot] = peek(0);
        break;
      }
      case OpCode::CALL:
      {
        int argCount = readByte(frame);
        if (!callValue(peek(argCount), argCount))
        {
          return InterpretResult::RUNTIME_ERROR;
        }
        frame = &this->frames[this->frameCount - 1];
        break;
      }
      case OpCode::RETURN:
      {
        Value result = pop();
        this->frameCount--;
        if (this->frameCount == 0)
        {
          pop();
          return InterpretResult::OK;
        }

        this->stackTop = frame->slots;
        push(result);
        frame = &this->frames[this->frameCount - 1];
        break;
      }
      case OpCode::CLOSURE: break;
      case OpCode::CONCAT: binaryOp(Operator::CONCAT); break;
      default: break;
    }
  }
}

bool VM::callValue(Value callee, int argCount)
{
  if (std::holds_alternative<Object*>(callee))
  {
    Object* obj = std::get<Object*>(callee);
    switch (obj->type)
    {
      case ObjType::FUNCTION: return call(static_cast<ObjectFunction*>(obj), argCount);
      case ObjType::NATIVE:
      {
        NativeFn native = static_cast<ObjectNative*>(obj)->function;
        Value result = native(argCount, stackTop - argCount);
        stackTop -= argCount + 1;
        push(result);
        return true;
      }
      default: break;
    }
  }
  throw std::runtime_error("Can only call functions.");
  return false;
}

bool VM::call(ObjectFunction* function, int argCount)
{
  CallFrame* frame = &this->frames[this->frameCount++];
  frame->function = function;
  frame->ip = function->chunk.code.data();
  frame->slots = this->stackTop - argCount - 1;
  return true;
}

bool VM::isFalsey(const Value& value)
{
  return std::holds_alternative<NilValue>(value) ||
         (std::holds_alternative<bool>(value) && !std::get<bool>(value));
}

Value VM::peek(int distance)
{
  return *(stackTop - 1 - distance);
}

void VM::defineNative(const std::string& name, NativeFn function)
{
  globals[name] = newNative(function);
}
