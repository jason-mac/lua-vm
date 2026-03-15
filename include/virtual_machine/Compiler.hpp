#pragma once
#include "Common.hpp"
#include "Object.hpp"
#include "expressions/BinaryExpr.hpp"
#include "expressions/CallExpr.hpp"
#include "expressions/Expr.hpp"
#include "expressions/FieldExpr.hpp"
#include "expressions/FunctionExpr.hpp"
#include "expressions/IndexExpr.hpp"
#include "expressions/LiteralExpr.hpp"
#include "expressions/NameExpr.hpp"
#include "expressions/UnaryExpr.hpp"
#include "statements/AssignStmt.hpp"
#include "statements/BlockStmt.hpp"
#include "statements/BreakStmt.hpp"
#include "statements/DoStmt.hpp"
#include "statements/ExpressionStmt.hpp"
#include "statements/ForEachStmt.hpp"
#include "statements/ForRangeStmt.hpp"
#include "statements/FunctionStmt.hpp"
#include "statements/IfStmt.hpp"
#include "statements/LocalStmt.hpp"
#include "statements/RepeatStmt.hpp"
#include "statements/ReturnStmt.hpp"
#include "statements/Stmt.hpp"
#include "statements/WhileStmt.hpp"
#include "virtual_machine/Chunk.hpp"
#include "virtual_machine/OpCode.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class CompileError : public std::runtime_error
{
public:
  CompileError(const std::string& message) : std::runtime_error(message) {}
};

struct Local
{
  std::string name;
  int depth = 0;
};

enum class FunctionType
{
  FUNCTION,
  SCRIPT
};

class Compiler : public Visitor
{
public:
  Compiler(FunctionType type = FunctionType::SCRIPT,
           const std::string& name = "",
           Compiler* compiler = nullptr)
  {
    function = newFunction(name, 0);
    functionType = type;
    locals.push_back({"", 0});
    this->enclosing = compiler;
  }

  ObjectFunction* compile(const std::vector<std::unique_ptr<Stmt>>& stmts);

private:
  Compiler* enclosing;
  ObjectFunction* function;
  FunctionType functionType;
  int currentLine = 0;

  std::vector<Local> locals;
  int scopeDepth = 0;

private:
  uint8_t makeConstant(Value value);

  // statements
  void visitLocalStmt(const LocalStmt* stmt) override;
  void visitAssignStmt(const AssignStmt* stmt) override;
  void visitIfStmt(const IfStmt* stmt) override;
  void visitWhileStmt(const WhileStmt* stmt) override;
  void visitRepeatStmt(const RepeatStmt* stmt) override;
  void visitDoStmt(const DoStmt* stmt) override;
  void visitForRangeStmt(const ForRangeStmt* stmt) override;
  void visitForEachStmt(const ForEachStmt* stmt) override;
  void visitFunctionStmt(const FunctionStmt* stmt) override;
  void visitReturnStmt(const ReturnStmt* stmt) override;
  void visitBreakStmt(const BreakStmt* stmt) override;
  void visitBlockStmt(const BlockStmt* stmt) override;
  void visitExpressionStmt(const ExpressionStmt* stmt) override;

  // expressions
  void visitLiteralExpr(const LiteralExpr* expr) override;
  void visitBinaryExpr(const BinaryExpr* expr) override;
  void visitUnaryExpr(const UnaryExpr* expr) override;
  void visitNameExpr(const NameExpr* expr) override;
  void visitCallExpr(const CallExpr* expr) override;
  void visitFunctionExpr(const FunctionExpr* expr) override;
  void visitFieldExpr(const FieldExpr* expr) override;
  void visitIndexExpr(const IndexExpr* expr) override;

  void emitByte(Byte byte);
  void emitConstant(Byte byte, Value value);
  int emitJump(OpCode op);
  void patchJump(int16_t jumpPos);
  void emitLoop(int loopStart);

  template <typename... Bytes> void emitBytes(Bytes... bytes)
  {
    (emitByte(bytes), ...);
  }

  uint8_t addConstant(Value value);

  void beginScope();
  void endScope();
  int resolveLocal(const std::string& name);
  void addLocal(const std::string& name);
  void markInitialized();
};
