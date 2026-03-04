#pragma once
#include "Stmt.hpp"
#include <memory>
#include <vector>

class BlockStmt : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> statements;
  BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
      : statements(std::move(stmts)) {}
};
