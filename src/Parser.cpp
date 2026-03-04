#include "Parser.hpp"
#include "expressions/BinaryExpr.hpp"
#include "expressions/LiteralExpr.hpp"
#include "expressions/NameExpr.hpp"
#include "expressions/UnaryExpr.hpp"
#include "statements/AssignStmt.hpp"
#include "statements/BlockStmt.hpp"
#include "statements/ExpressionStmt.hpp"
#include "statements/IfStmt.hpp"
#include "statements/LocalStmt.hpp"
#include "statements/ReturnStmt.hpp"
#include "statements/WhileStmt.hpp"
#include <iostream>
#include <memory>
#include <variant>

void Parser::sync() {
  advance();
  while (!done()) {
    switch (peek().type) {
    case TokenType::LOCAL:
    case TokenType::FUNCTION:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::FOR:
    case TokenType::REPEAT:
    case TokenType::DO:
    case TokenType::RETURN:
    case TokenType::BREAK:
    case TokenType::END:
    case TokenType::ELSE:
    case TokenType::ELSEIF:
    case TokenType::UNTIL:
      return;
    default:
      break;
    }
    advance();
  }
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!done()) {
    statements.push_back(declaration());
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
  try {
    if (match(TokenType::LOCAL)) {
      return localDeclaration();
    }
    return statement();
  } catch (ParseError &error) {
    std::cerr << "ParseError: " << error.what() << " at line " << error.line
              << "\n";
    sync();
  }
  return nullptr;
}

Token Parser::previous() const { return this->tokens[current - 1]; }

Token Parser::advance() {
  if (!done()) {
    current++;
  }
  return previous();
}

bool Parser::check(TokenType type) {
  return done() ? false : peek().type == type;
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match(TokenType::IF)) {
    return ifStatement(false);
  }
  if (match(TokenType::FOR)) {
    return forStatement();
  }
  if (match(TokenType::RETURN)) {
    return returnStatement();
  }
  if (match(TokenType::WHILE)) {
    return whileStatement();
  }
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::returnStatement() {
  Token keyword = previous();
  std::unique_ptr<Expr> value = nullptr;
  if (!done() && !check(TokenType::END) && !check(TokenType::ELSE) &&
      !check(TokenType::ELSEIF)) {
    value = expression();
  }
  return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
}

std::unique_ptr<Stmt> Parser::forStatement() {
  // pass
  return nullptr;
}

std::unique_ptr<Stmt> Parser::ifStatement(bool isElseIf) {
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::THEN, "Expect 'then' after condition.");
  std::unique_ptr<Stmt> thenBranch = std::make_unique<BlockStmt>(block());
  std::unique_ptr<Stmt> elseBranch = nullptr;
  if (match(TokenType::ELSEIF)) {
    elseBranch = ifStatement(true);
  } else if (match(TokenType::ELSE)) {
    elseBranch = std::make_unique<BlockStmt>(block());
  }
  if (!isElseIf) {
    consume(TokenType::END, "Expect 'end' after if block.");
  }
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::DO, "Expect 'do' after condition");
  std::unique_ptr<BlockStmt> body = std::make_unique<BlockStmt>(block());
  consume(TokenType::END, "Exppect 'end' after body");
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  std::unique_ptr<Expr> expr = expression();
  if (match(TokenType::EQUAL)) {
    std::unique_ptr<Expr> value = expression();
    auto *name = dynamic_cast<NameExpr *>(expr.get());
    if (!name) {
      throw ParseError("Invalid assignment target.", peek().line);
    }
    return std::make_unique<AssignStmt>(name->name, std::move(value));
  }
  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::localDeclaration() {
  Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
  std::unique_ptr<Expr> initializer = nullptr;
  if (match(TokenType::EQUAL)) {
    initializer = expression();
  }
  return std::make_unique<LocalStmt>(name, std::move(initializer));
}
std::unique_ptr<Expr> Parser::expression() { return or_(); }

Token Parser::consume(TokenType type, std::string message) {
  if (check(type)) {
    return advance();
  } else {
    throw ParseError(message, peek().line);
  }
}

bool Parser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::match(TokenType type) { return match({type}); }
bool Parser::match(TokenType type1, TokenType type2) {
  return match({type1, type2});
}

bool Parser::match(TokenType type1, TokenType type2, TokenType type3) {
  return match({type1, type2, type3});
}

bool Parser::match(TokenType type1, TokenType type2, TokenType type3,
                   TokenType type4) {
  return match({type1, type2, type3, type4});
}

Token Parser::peek() { return tokens[current]; }
bool Parser::done() { return peek().type == TokenType::EOF_LUA; }

std::unique_ptr<Expr> Parser::unary() {
  if (match(TokenType::NOT, TokenType::MINUS, TokenType::HASH)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<UnaryExpr>(oper, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::NUMBER)) {
    return std::make_unique<LiteralExpr>(previous().literal);
  }
  if (match(TokenType::STRING)) {
    return std::make_unique<LiteralExpr>(previous().literal);
  }
  if (match(TokenType::TRUE)) {
    return std::make_unique<LiteralExpr>(true);
  }
  if (match(TokenType::FALSE)) {
    return std::make_unique<LiteralExpr>(false);
  }
  if (match(TokenType::NIL)) {
    return std::make_unique<LiteralExpr>(std::monostate{});
  }
  if (match(TokenType::IDENTIFIER)) {
    return std::make_unique<NameExpr>(previous());
  }
  if (match(TokenType::LEFT_PAREN)) {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
  }
  throw ParseError("Expected expression.", peek().line);
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (match(TokenType::SLASH, TokenType::STAR, TokenType::PERCENT)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::or_() {
  std::unique_ptr<Expr> expr = and_();
  while (match(TokenType::OR)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = and_();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::and_() {
  std::unique_ptr<Expr> expr = equality();
  while (match(TokenType::AND)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = equality();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();
  while (match(TokenType::EQUAL_EQUAL, TokenType::TILDE_EQUAL)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();
  while (match(TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL,
               TokenType::GREATER_EQUAL)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  while (match(TokenType::MINUS, TokenType::PLUS)) {
    Token oper = previous();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(oper),
                                        std::move(right));
  }
  return expr;
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (!done() && !check(TokenType::END) && !check(TokenType::ELSE) &&
         !check(TokenType::ELSEIF) && !check(TokenType::UNTIL)) {
    stmts.push_back(declaration());
  }
  return stmts;
}
