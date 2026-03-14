#pragma once
#include "Expr.hpp"
#include <memory>

class IndexExpr : public Expr
{
public:
  std::unique_ptr<Expr> object;
  std::unique_ptr<Expr> index;

  IndexExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index)
      : object(std::move(object)), index(std::move(index))
  {
  }
};
