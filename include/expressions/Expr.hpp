#pragma once
#include "Visitor.hpp"

class Expr
{
public:
  virtual ~Expr() = default;
  virtual void accept(Visitor* v) = 0;
};
