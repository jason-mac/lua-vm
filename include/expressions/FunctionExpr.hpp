#pragma once
#include "Common.hpp"
#include "Expr.hpp"
#include "statements/Stmt.hpp"
#include "tokens/Token.hpp"
#include <memory>
#include <vector>

class FunctionExpr : public Expr
{
public:
  std::vector<Token> params;
  std::unique_ptr<Stmt> body;

  FunctionExpr(std::vector<Token> params, std::unique_ptr<Stmt> body)
      : params(std::move(params)), body(std::move(body))
  {
  }

  void accept(Visitor* v) override
  {
    v->visitFunctionExpr(this);
  }
};
