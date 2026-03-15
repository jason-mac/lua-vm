#pragma once
#include "Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>

class UnaryExpr : public Expr
{
public:
  Token operator_;
  std::unique_ptr<Expr> rightOperand;
  UnaryExpr(Token o, std::unique_ptr<Expr> r) : operator_(std::move(o)), rightOperand(std::move(r))
  {
  }

  void accept(Visitor* v) override
  {
    v->visitUnaryExpr(this);
  }
};
