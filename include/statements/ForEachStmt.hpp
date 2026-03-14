#pragma once
#include "Stmt.hpp"
#include "expressions/Expr.hpp"
#include "tokens/Token.hpp"
#include <memory>
#include <vector>

class ForEachStmt : public Stmt
{
public:
  std::vector<Token> names;
  std::vector<std::unique_ptr<Expr>> explist;
  std::unique_ptr<Stmt> body;

  ForEachStmt(std::vector<Token> names,
              std::vector<std::unique_ptr<Expr>> explist,
              std::unique_ptr<Stmt> body)
      : names(std::move(names)), explist(std::move(explist)), body(std::move(body))
  {
  }
};
