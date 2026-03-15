#include "virtual_machine/Compiler.hpp"
#include "expressions/BinaryExpr.hpp"
#include "expressions/LiteralExpr.hpp"
#include "expressions/NameExpr.hpp"
#include "statements/AssignStmt.hpp"
#include "statements/LocalStmt.hpp"
#include "virtual_machine/OpCode.hpp"
#include <iostream>

/* TODO:
 * implement
 * 1) visitLocalStmt
 * 2) visitNameExpr
 * 3) visitBreakStmt
 * 4) visitFunctionExpr
 * 5) visitFunctionStmt
 * 6) visitFieldExpr
 * 7) visitIndexExpr
 * 8) visitForRangeStmt
 * 9) visitForEachStmt
 * 10) visitDoStmt
 */

Chunk Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& stmts)
{
  for (const auto& stmt : stmts)
  {
    stmt->accept(this);
  }
  return chunk;
}

uint8_t Compiler::makeConstant(Value value)
{
  int index = chunk.addConstant(value);
  if (index > UINT8_MAX)
  {
    throw CompileError("Too many constants in one chunk");
  }
  return (uint8_t)index;
}

void Compiler::emitByte(Byte byte)
{
  chunk.code.push_back(byte);
  chunk.lines.push_back(0);
}

void Compiler::visitLiteralExpr(const LiteralExpr* expr)
{
  Value value;
  if (std::holds_alternative<double>(expr->value)) value = std::get<double>(expr->value);
  else if (std::holds_alternative<std::string>(expr->value))
    value = std::get<std::string>(expr->value);
  else if (std::holds_alternative<bool>(expr->value)) value = std::get<bool>(expr->value);
  else value = NilValue{};
  emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(value));
}

void Compiler::visitReturnStmt(const ReturnStmt* stmt)
{
  if (stmt->value)
  {
    stmt->value->accept(this);
  }
  emitByte((Byte)OpCode::RETURN);
}

void Compiler::visitUnaryExpr(const UnaryExpr* expr)
{
  expr->rightOperand->accept(this);
  switch (expr->operator_.type)
  {
    case TokenType::MINUS: emitByte((Byte)OpCode::NEG); break;
    case TokenType::NOT: emitByte((Byte)OpCode::NOT); break;
    case TokenType::HASH: emitByte((Byte)OpCode::LEN); break;
    default: throw CompileError("Unknown operator for unary expression");
  }
}

void Compiler::visitBinaryExpr(const BinaryExpr* expr)
{
  expr->left->accept(this);
  expr->right->accept(this);
  switch (expr->op.type)
  {
    case TokenType::PLUS: emitByte((Byte)OpCode::ADD); break;
    case TokenType::MINUS: emitByte((Byte)OpCode::SUB); break;
    case TokenType::SLASH: emitByte((Byte)OpCode::DIV); break;
    case TokenType::STAR: emitByte((Byte)OpCode::MUL); break;
    case TokenType::PERCENT: emitByte((Byte)OpCode::MOD); break;
    case TokenType::CARET: emitByte((Byte)OpCode::POW); break;
    case TokenType::DOT_DOT: emitByte((Byte)OpCode::CONCAT); break;
    case TokenType::EQUAL_EQUAL: emitByte((Byte)OpCode::EQ); break;
    case TokenType::TILDE_EQUAL: emitBytes((Byte)OpCode::EQ, (Byte)OpCode::NOT); break;
    case TokenType::LESS: emitByte((Byte)OpCode::LT); break;
    case TokenType::LESS_EQUAL: emitByte((Byte)OpCode::LE); break;
    case TokenType::GREATER: emitBytes((Byte)OpCode::LE, (Byte)OpCode::NOT); break;
    case TokenType::GREATER_EQUAL: emitBytes((Byte)OpCode::LT, (Byte)OpCode::NOT); break;
    case TokenType::AND: emitByte((Byte)OpCode::AND); break;
    case TokenType::OR: emitByte((Byte)OpCode::OR); break;
    default: throw CompileError("Unknown binary operator");
  }
}
void Compiler::visitCallExpr(const CallExpr* expr)
{

  expr->callee->accept(this);

  for (const auto& arg : expr->arguments)
  {
    arg->accept(this);
  }

  emitBytes((Byte)OpCode::CALL, (Byte)expr->arguments.size());
}

int Compiler::emitJump(OpCode op)
{
  emitByte((Byte)op);
  emitByte(0);
  emitByte(0);
  return chunk.code.size() - 2;
}

void Compiler::patchJump(int16_t jumpPos)
{
  int16_t offset = chunk.code.size() - jumpPos - 4;
  chunk.code[jumpPos] = (offset >> 8) & 0xFF;
  chunk.code[jumpPos + 1] = (offset) & 0xFF;
}

void Compiler::visitIfStmt(const IfStmt* stmt)
{
  stmt->condition->accept(this);
  int16_t jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  stmt->thenBranch->accept(this);
  if (stmt->elseBranch)
  {
    int16_t elseJumpPos = emitJump(OpCode::JMP);
    patchJump(jumpPos);
    stmt->elseBranch->accept(this);
    patchJump(elseJumpPos);
  }
  else
  {
    patchJump(jumpPos);
  }
}

void Compiler::emitLoop(int loopStart)
{
  emitByte((Byte)OpCode::JMP);
  int16_t offset = -(chunk.code.size() - loopStart + 3);
  emitByte((offset >> 8) & 0xFF);
  emitByte(offset & 0xFF);
}

void Compiler::visitWhileStmt(const WhileStmt* stmt)
{
  int loopStart = this->chunk.code.size();
  stmt->condition->accept(this);
  int16_t jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  stmt->body->accept(this);
  emitLoop(loopStart);
  patchJump(jumpPos);
}

void Compiler::visitLocalStmt(const LocalStmt* stmt) {}
void Compiler::visitAssignStmt(const AssignStmt* stmt)
{
  stmt->value->accept(this);
  uint8_t index = makeConstant(stmt->name.lexeme);
  emitBytes((Byte)OpCode::SET_GLOBAL, index);
}

void Compiler::visitRepeatStmt(const RepeatStmt* stmt)
{
  int loopStart = this->chunk.code.size();
  stmt->body->accept(this);
  stmt->condition->accept(this);
  emitByte((Byte)OpCode::NOT);
  int16_t jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  emitLoop(loopStart);
  patchJump(jumpPos);
}

void Compiler::visitDoStmt(const DoStmt* stmt)
{
  stmt->body->accept(this);
}

void Compiler::visitForRangeStmt(const ForRangeStmt* stmt) {}
void Compiler::visitForEachStmt(const ForEachStmt* stmt) {}
void Compiler::visitFunctionStmt(const FunctionStmt* stmt) {}
void Compiler::visitBreakStmt(const BreakStmt* stmt) {}
void Compiler::visitBlockStmt(const BlockStmt* stmt)
{
  for (const auto& s : stmt->statements)
  {
    s->accept(this);
  }
}

void Compiler::visitExpressionStmt(const ExpressionStmt* stmt)
{
  stmt->expression->accept(this);
}

void Compiler::visitNameExpr(const NameExpr* expr) {}
void Compiler::visitFunctionExpr(const FunctionExpr* expr) {}
void Compiler::visitFieldExpr(const FieldExpr* expr) {}
void Compiler::visitIndexExpr(const IndexExpr* expr) {}
