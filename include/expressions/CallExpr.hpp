#pragma once
#include "Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>
#include <vector>

class CallExpr : public Expr
{
public:
  std::unique_ptr<Expr> callee;
  Token paren;
  std::vector<std::unique_ptr<Expr>> arguments;

  CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
      : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments))
  {
  }

  void accept(Visitor* v) override
  {
    v->visitCallExpr(this);
  }
};
