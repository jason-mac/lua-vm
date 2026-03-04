#pragma once
#include "Stmt.hpp"
#include "expressions/Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>

class AssignStmt : public Stmt {
public:
  Token name;
  std::unique_ptr<Expr> value;
  AssignStmt(Token name, std::unique_ptr<Expr> value)
      : name(std::move(name)), value(std::move(value)) {}
};
