#pragma once
#include "Object.hpp"
#include "virtual_machine/Chunk.hpp"
#include <string>

ObjectFunction* newFunction(const std::string& name, int arity)
{
  ObjectFunction* function = new ObjectFunction();
  function->arity = arity;
  function->name = name;
  return function;
}

ObjectNative* newNative(NativeFn function)
{
  ObjectNative* native = new ObjectNative();
  native->type = ObjType::NATIVE;
  native->function = function;
  return native;
}

ObjectClosure* newClosure(ObjectFunction* fn)
{
  return nullptr;
}
ObjectUpvalue* newUpvalue(Value* slot)
{
  return nullptr;
}
