#pragma once
#include "Stmt.hpp"
#include "expressions/Expr.hpp"
#include <memory>

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> cond,
           std::unique_ptr<Stmt> thenB,
           std::unique_ptr<Stmt> elseB)
        : condition(std::move(cond)),
          thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}
};
