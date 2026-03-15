#pragma once
#include "virtual_machine/Chunk.hpp"
#include "virtual_machine/Value.hpp"
#include <cstdint>
#include <functional>
#include <string>

enum class ObjType
{
  FUNCTION,
  NATIVE,
  STRING,
  UPVALUE,
  CLOSURE,
  TABLE
};

struct Object
{
  ObjType type;
  bool isMarked = false;
  Object* next = nullptr;
};

struct ObjectFunction : public Object
{
  ObjectFunction()
  {
    type = ObjType::FUNCTION;
  }
  int arity = 0;
  int upvalueCount = 0;
  Chunk chunk;
  std::string name;
};

ObjectFunction* newFunction(const std::string& name = "", int arity = 0);

using NativeFn = std::function<Value(int, Value*)>;

struct ObjectNative : public Object
{
  NativeFn function;
};

ObjectNative* newNative(NativeFn function);

struct ObjectUpvalue : public Object
{
  Value* location = nullptr;
  Value closed;
  ObjectUpvalue* nextUpvalue = nullptr;
};

struct ObjectClosure : public Object
{
  ObjectFunction* function = nullptr;
  std::vector<ObjectUpvalue*> upvalues;
};

// helpers
static inline bool isObjType(Value value, ObjType type)
{
  if (!std::holds_alternative<Object*>(value)) return false;
  return std::get<Object*>(value)->type == type;
}

static inline bool isString(Value value)
{
  return isObjType(value, ObjType::STRING);
}
static inline bool isFunction(Value value)
{
  return isObjType(value, ObjType::FUNCTION);
}
static inline bool isClosure(Value value)
{
  return isObjType(value, ObjType::CLOSURE);
}
static inline bool isNative(Value value)
{
  return isObjType(value, ObjType::NATIVE);
}

static inline ObjectFunction* asFunction(Value value)
{
  return static_cast<ObjectFunction*>(std::get<Object*>(value));
}
static inline ObjectClosure* asClosure(Value value)
{
  return static_cast<ObjectClosure*>(std::get<Object*>(value));
}
static inline NativeFn asNative(Value value)
{
  return static_cast<ObjectNative*>(std::get<Object*>(value))->function;
}
