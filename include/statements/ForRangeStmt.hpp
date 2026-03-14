#pragma once
#include "Stmt.hpp"
#include "expressions/Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>

class ForRangeStmt : public Stmt
{
public:
  Token name;
  std::unique_ptr<Expr> start;
  std::unique_ptr<Expr> stop;
  std::unique_ptr<Expr> step;
  std::unique_ptr<Stmt> body;

  ForRangeStmt(Token name,
               std::unique_ptr<Expr> start,
               std::unique_ptr<Expr> stop,
               std::unique_ptr<Expr> step,
               std::unique_ptr<Stmt> body)
      : name(std::move(name)), start(std::move(start)), stop(std::move(stop)),
        step(std::move(step)), body(std::move(body))
  {
  }
};
