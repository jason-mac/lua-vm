#pragma once
#include "Stmt.hpp"
#include "expressions/Expr.hpp"
#include <memory>

class ExpressionStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;

  ExpressionStmt(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}
};
