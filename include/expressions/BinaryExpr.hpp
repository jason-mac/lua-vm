#pragma once
#include "Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>

class BinaryExpr : public Expr
{
public:
  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
  BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
      : left(std::move(l)), op(std::move(o)), right(std::move(r))
  {
  }

  void accept(Visitor* v) override
  {
    v->visitBinaryExpr(this);
  }
};
