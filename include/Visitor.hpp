#pragma once

class LocalStmt;
class AssignStmt;
class IfStmt;
class WhileStmt;
class RepeatStmt;
class DoStmt;
class ForRangeStmt;
class ForEachStmt;
class FunctionStmt;
class ReturnStmt;
class BreakStmt;
class BlockStmt;
class ExpressionStmt;

class LiteralExpr;
class BinaryExpr;
class UnaryExpr;
class NameExpr;
class CallExpr;
class FunctionExpr;
class FieldExpr;
class IndexExpr;

class Visitor
{
public:
  virtual ~Visitor() = default;

  // statements
  virtual void visitLocalStmt(const LocalStmt* stmt) = 0;
  virtual void visitAssignStmt(const AssignStmt* stmt) = 0;
  virtual void visitIfStmt(const IfStmt* stmt) = 0;
  virtual void visitWhileStmt(const WhileStmt* stmt) = 0;
  virtual void visitRepeatStmt(const RepeatStmt* stmt) = 0;
  virtual void visitDoStmt(const DoStmt* stmt) = 0;
  virtual void visitForRangeStmt(const ForRangeStmt* stmt) = 0;
  virtual void visitForEachStmt(const ForEachStmt* stmt) = 0;
  virtual void visitFunctionStmt(const FunctionStmt* stmt) = 0;
  virtual void visitReturnStmt(const ReturnStmt* stmt) = 0;
  virtual void visitBreakStmt(const BreakStmt* stmt) = 0;
  virtual void visitBlockStmt(const BlockStmt* stmt) = 0;
  virtual void visitExpressionStmt(const ExpressionStmt* stmt) = 0;

  // expressions
  virtual void visitLiteralExpr(const LiteralExpr* expr) = 0;
  virtual void visitBinaryExpr(const BinaryExpr* expr) = 0;
  virtual void visitUnaryExpr(const UnaryExpr* expr) = 0;
  virtual void visitNameExpr(const NameExpr* expr) = 0;
  virtual void visitCallExpr(const CallExpr* expr) = 0;
  virtual void visitFunctionExpr(const FunctionExpr* expr) = 0;
  virtual void visitFieldExpr(const FieldExpr* expr) = 0;
  virtual void visitIndexExpr(const IndexExpr* expr) = 0;
};
