#pragma once
#include "Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>

class FieldExpr : public Expr
{
public:
  std::unique_ptr<Expr> object;
  Token field;

  FieldExpr(std::unique_ptr<Expr> object, Token field)
      : object(std::move(object)), field(std::move(field))
  {
  }
  void accept(Visitor* v) override
  {
    v->visitFieldExpr(this);
  }
};
