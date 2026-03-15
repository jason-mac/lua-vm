#pragma once
#include "Expr.hpp"
#include "tokens/Token.hpp"

class NameExpr : public Expr
{
public:
  Token name;
  NameExpr(Token name) : name(std::move(name)) {}

  void accept(Visitor* v) override
  {
    v->visitNameExpr(this);
  }
};
