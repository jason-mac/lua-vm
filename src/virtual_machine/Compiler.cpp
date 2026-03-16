#include "virtual_machine/Compiler.hpp"
#include "Debug.hpp"
#include "expressions/BinaryExpr.hpp"
#include "expressions/LiteralExpr.hpp"
#include "expressions/NameExpr.hpp"
#include "statements/AssignStmt.hpp"
#include "statements/LocalFunctionStmt.hpp"
#include "statements/LocalStmt.hpp"
#include "virtual_machine/OpCode.hpp"
#include <iostream>

/* TODO:
 * implement
 * 1) visitBreakStmt - emit JMP with placeholder, save position for patching at loop end
 * 2) visitFunctionExpr - compile function body into new chunk, emit CLOSURE
 * 3) visitFunctionStmt - same as visitFunctionExpr but also emit SET_GLOBAL for name
 * 4) visitFieldExpr - compile object, emit GET_TABLE with field name as constant
 * 5) visitIndexExpr - compile object, compile index, emit GET_TABLE
 * 6) visitForEachStmt - compile explist, emit loop with iterator protocol
 */

ObjectFunction* Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& stmts)
{
  for (const auto& stmt : stmts)
  {
    stmt->accept(this);
  }
  emitByte((Byte)OpCode::LOAD_NIL);
  emitByte((Byte)OpCode::RETURN);
  return function;
}

uint8_t Compiler::makeConstant(Value value)
{
  int index = function->chunk.addConstant(value);
  if (index > UINT8_MAX)
  {
    throw CompileError("Too many constants in one chunk");
  }
  return (uint8_t)index;
}

void Compiler::emitByte(Byte byte)
{
  function->chunk.code.push_back(byte);
  function->chunk.lines.push_back(currentLine);
}

void Compiler::visitLiteralExpr(const LiteralExpr* expr)
{
  Value value;
  if (std::holds_alternative<double>(expr->value))
  {
    value = std::get<double>(expr->value);
  }
  else if (std::holds_alternative<std::string>(expr->value))
  {
    value = std::get<std::string>(expr->value);
  }
  else if (std::holds_alternative<bool>(expr->value))
  {
    value = std::get<bool>(expr->value);
  }
  else
  {
    value = NilValue{};
  }
  emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(value));
}

void Compiler::visitReturnStmt(const ReturnStmt* stmt)
{
  currentLine = stmt->keyword.line;
  if (stmt->value)
  {
    stmt->value->accept(this);
  }
  emitByte((Byte)OpCode::RETURN);
}

void Compiler::visitUnaryExpr(const UnaryExpr* expr)
{
  currentLine = expr->operator_.line;
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
  currentLine = expr->op.line;
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
  currentLine = expr->paren.line;
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
  return function->chunk.code.size() - 2;
}

void Compiler::patchJump(int16_t jumpPos)
{
  int16_t offset = function->chunk.code.size() - jumpPos - 2;
  function->chunk.code[jumpPos] = (offset >> 8) & 0xFF;
  function->chunk.code[jumpPos + 1] = (offset) & 0xFF;
}

void Compiler::visitIfStmt(const IfStmt* stmt)
{
  stmt->condition->accept(this);

  int16_t thenJump = emitJump(OpCode::JMP_IF_FALSE);
  emitByte((Byte)OpCode::POP);

  stmt->thenBranch->accept(this);

  int16_t endJump = emitJump(OpCode::JMP);

  patchJump(thenJump);
  emitByte((Byte)OpCode::POP);

  if (stmt->elseBranch)
  {
    stmt->elseBranch->accept(this);
  }

  patchJump(endJump);
}

void Compiler::emitLoop(int loopStart)
{
  emitByte((Byte)OpCode::JMP);
  int16_t offset = loopStart - function->chunk.code.size() - 2;
  emitByte((uint8_t)((offset >> 8) & 0xFF));
  emitByte((uint8_t)(offset & 0xFF));
}

void Compiler::visitWhileStmt(const WhileStmt* stmt)
{
  int loopStart = this->function->chunk.code.size();
  stmt->condition->accept(this);
  int16_t jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  stmt->body->accept(this);
  emitLoop(loopStart);
  patchJump(jumpPos);
}

void Compiler::addLocal(const std::string& name)
{
  locals.push_back({name, -1});
}

void Compiler::visitLocalFunctionStmt(const LocalFunctionStmt* stmt)
{
  addLocal(stmt->name.lexeme);
  markInitialized();

  stmt->function->accept(this);
}

int Compiler::resolveLocal(const std::string& name)
{
  for (int i = (int)this->locals.size() - 1; i >= 0; i--)
  {
    Local* local = &this->locals[i];
    if (name == local->name)
    {
      if (local->depth == -1)
      {
        std::cout << "initializer error for: " << name << std::endl;
        throw CompileError("Can't read local variables in its own initializer");
      }
      return i;
    }
  }
  return -1;
}

void Compiler::markInitialized()
{
  this->locals.back().depth = this->scopeDepth;
}

void Compiler::visitLocalStmt(const LocalStmt* stmt)
{

  addLocal(stmt->name.lexeme);
  if (stmt->value)
  {
    stmt->value->accept(this);
  }
  else
  {
    emitByte((Byte)OpCode::LOAD_NIL);
  }
  markInitialized();
}

void Compiler::visitAssignStmt(const AssignStmt* stmt)
{
  currentLine = stmt->name.line;
  stmt->value->accept(this);
  int slot = resolveLocal(stmt->name.lexeme);
  if (slot != -1)
  {
    emitBytes((Byte)OpCode::SET_LOCAL, (Byte)slot);
  }
  else
  {
    uint8_t index = makeConstant(stmt->name.lexeme);
    emitBytes((Byte)OpCode::SET_GLOBAL, index);
  }
  emitByte((Byte)OpCode::POP);
}

void Compiler::visitRepeatStmt(const RepeatStmt* stmt)
{
  int loopStart = this->function->chunk.code.size();
  stmt->body->accept(this);
  stmt->condition->accept(this);
  emitByte((Byte)OpCode::NOT);
  int16_t jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  emitLoop(loopStart);
  patchJump(jumpPos);
}

void Compiler::visitDoStmt(const DoStmt* stmt)
{
  beginScope();
  stmt->body->accept(this);
  endScope();
}

void Compiler::visitForRangeStmt(const ForRangeStmt* stmt)
{
  beginScope();

  addLocal(stmt->name.lexeme);
  stmt->start->accept(this);
  markInitialized();

  addLocal("_stop");
  stmt->stop->accept(this);
  markInitialized();

  addLocal("_step");
  if (stmt->step) stmt->step->accept(this);
  else emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(1.0));
  markInitialized();

  int loopStart = function->chunk.code.size();

  int iSlot = resolveLocal(stmt->name.lexeme);
  int stopSlot = resolveLocal("_stop");
  emitBytes((Byte)OpCode::GET_LOCAL, (Byte)iSlot);
  emitBytes((Byte)OpCode::GET_LOCAL, (Byte)stopSlot);
  emitByte((Byte)OpCode::LE);
  int jumpPos = emitJump(OpCode::JMP_IF_FALSE);
  emitByte((Byte)OpCode::POP);

  stmt->body->accept(this);

  int stepSlot = resolveLocal("_step");
  emitBytes((Byte)OpCode::GET_LOCAL, (Byte)iSlot);
  emitBytes((Byte)OpCode::GET_LOCAL, (Byte)stepSlot);
  emitByte((Byte)OpCode::ADD);
  emitBytes((Byte)OpCode::SET_LOCAL, (Byte)iSlot);
  emitByte((Byte)OpCode::POP);

  emitLoop(loopStart);
  patchJump(jumpPos);
  emitByte((Byte)OpCode::POP);

  endScope();
}

void Compiler::visitForEachStmt(const ForEachStmt* stmt) {}

void Compiler::visitFunctionStmt(const FunctionStmt* stmt)
{
  Compiler fnCompiler(FunctionType::FUNCTION, stmt->name.lexeme, this);
  fnCompiler.beginScope();
  for (auto& param : stmt->function->params)
  {
    fnCompiler.addLocal(param.lexeme);
    fnCompiler.markInitialized();
    fnCompiler.function->arity++;
  }
  stmt->function->body->accept(&fnCompiler);
  fnCompiler.emitByte((Byte)OpCode::RETURN);
  ObjectFunction* fn = fnCompiler.function;

  if (scopeDepth > 0)
  {
    emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(fn));
    addLocal(stmt->name.lexeme);
    markInitialized();
  }
  else
  {
    emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(fn));
    uint8_t index = makeConstant(stmt->name.lexeme);
    emitBytes((Byte)OpCode::SET_GLOBAL, index);
    emitByte((Byte)OpCode::POP);
  }
}
void Compiler::visitBreakStmt(const BreakStmt* stmt) {}

void Compiler::beginScope()
{
  this->scopeDepth++;
}

void Compiler::endScope()
{
  scopeDepth--;
  while (!locals.empty() && locals.back().depth > scopeDepth)
  {
    emitByte((Byte)OpCode::POP);
    locals.pop_back();
  }
}

void Compiler::visitBlockStmt(const BlockStmt* stmt)
{
  beginScope();
  for (const auto& s : stmt->statements)
  {
    s->accept(this);
  }
  endScope();
}

void Compiler::visitExpressionStmt(const ExpressionStmt* stmt)
{
  stmt->expression->accept(this);
  emitByte((Byte)OpCode::POP);
}

void Compiler::visitNameExpr(const NameExpr* expr)
{
  int slot = resolveLocal(expr->name.lexeme);
  if (slot != -1)
  {
    emitBytes((Byte)OpCode::GET_LOCAL, (Byte)slot);
  }
  else
  {
    uint8_t index = makeConstant(expr->name.lexeme);
    emitBytes((Byte)OpCode::GET_GLOBAL, index);
  }
}

void Compiler::visitFunctionExpr(const FunctionExpr* expr)
{
  Compiler fnCompiler(FunctionType::FUNCTION, "", this);
  fnCompiler.beginScope();

  for (auto& param : expr->params)
  {
    fnCompiler.addLocal(param.lexeme);
    fnCompiler.markInitialized();
    fnCompiler.function->arity++;
  }

  expr->body->accept(&fnCompiler);

  fnCompiler.emitByte((Byte)OpCode::LOAD_NIL);
  fnCompiler.emitByte((Byte)OpCode::RETURN);

  ObjectFunction* fn = fnCompiler.function;

  emitBytes((Byte)OpCode::LOAD_CONST, makeConstant(fn));
}

void Compiler::visitFieldExpr(const FieldExpr* expr) {}
void Compiler::visitIndexExpr(const IndexExpr* expr) {}
