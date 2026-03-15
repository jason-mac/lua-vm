#include "Parser.hpp"
#include "expressions/BinaryExpr.hpp"
#include "expressions/CallExpr.hpp"
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
#include "statements/WhileStmt.hpp"
#include "tokens/TokenType.hpp"

#include <initializer_list>
#include <iostream>
#include <memory>
#include <variant>

static bool isBlockCloser(TokenType type)
{
  return type == TokenType::END || type == TokenType::ELSE || type == TokenType::ELSEIF ||
         type == TokenType::UNTIL || type == TokenType::RETURN || type == TokenType::BREAK;
}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!done())
  {
    try
    {
      statements.push_back(statement());
    }
    catch (ParseError& error)
    {
      std::cerr << "ParseError: " << error.what() << " at line " << error.line << "\n";
      sync();
    }
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::statement()
{
  // stat ::= varlist `=` explist | functioncall | do block end | while exp do block end | repeat
  // block until exp | if exp then block end | for | function | local
  if (match(TokenType::LOCAL)) return localStatement();
  if (match(TokenType::WHILE)) return whileStatement();
  if (match(TokenType::REPEAT)) return repeatStatement();
  if (match(TokenType::DO)) return doStatement();
  if (match(TokenType::IF)) return ifStatement();
  if (match(TokenType::RETURN)) return returnStatement();
  if (match(TokenType::FOR)) return forStatement();
  if (match(TokenType::FUNCTION)) return functionStatement();

  return expressionStatement();
}

FuncBody Parser::parseFuncBody()
{
  consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
  std::vector<Token> params;
  if (!check(TokenType::RIGHT_PAREN))
  {
    do
    {
      if (params.size() >= 255)
        throw ParseError("Cannot have more than 255 parameters.", peek().line);
      params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name"));
    } while (match(TokenType::COMMA));
  }
  consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
  std::unique_ptr<Stmt> body = blockStatement();
  consume(TokenType::END, "Expected 'end' after function body");
  return FuncBody{std::move(params), std::move(body)};
}

std::unique_ptr<Stmt> Parser::forStatement()
{
  // stat ::= for Name `=` exp `,` exp [`,` exp] do block end | for namelist in explist do block
  // end
  Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'for'.");
  if (match(TokenType::EQUAL))
  {
    return forRangeStatement(name);
  }
  return forEachStatement(name);
}

std::unique_ptr<Stmt> Parser::forEachStatement(Token name)
{
  std::vector<Token> names = {name};
  while (match(TokenType::COMMA))
  {
    names.push_back(consume(TokenType::IDENTIFIER, "Expected name in namelist"));
  }
  consume(TokenType::IN, "Expected 'in' after namelist");
  std::vector<std::unique_ptr<Expr>> exprlist;

  do
  {
    exprlist.push_back(expression());
  } while (match(TokenType::COMMA));
  consume(TokenType::DO, "Expceted 'do' after explist");
  std::unique_ptr<Stmt> body = blockStatement();
  consume(TokenType::END, "Expected 'end' after body of for each statement");
  return std::make_unique<ForEachStmt>(std::move(names), std::move(exprlist), std::move(body));
}

std::unique_ptr<Stmt> Parser::forRangeStatement(Token name)
{
  std::unique_ptr<Expr> start = expression();
  consume(TokenType::COMMA, "Expected ',' after start value.");
  std::unique_ptr<Expr> stop = expression();
  std::unique_ptr<Expr> step = nullptr;
  if (match(TokenType::COMMA))
  {
    step = expression();
  }
  consume(TokenType::DO, "Expected 'do' after for declaration");
  std::unique_ptr<Stmt> body = blockStatement();
  consume(TokenType::END, "Expected 'end' after body of for statement");
  return std::make_unique<ForRangeStmt>(
      std::move(name), std::move(start), std::move(stop), std::move(step), std::move(body));
}

std::unique_ptr<Expr> Parser::functionExpression()
{
  FuncBody body = parseFuncBody();
  return std::make_unique<FunctionExpr>(std::move(body.params), std::move(body.body));
}

std::unique_ptr<Stmt> Parser::functionStatement()
{
  Token name = consume(TokenType::IDENTIFIER, "Expected function name after declaration");
  FuncBody body = parseFuncBody();
  auto func = std::make_unique<FunctionExpr>(std::move(body.params), std::move(body.body));
  return std::make_unique<FunctionStmt>(std::move(name), std::move(func));
}

std::unique_ptr<Stmt> Parser::returnStatement()
{
  // laststat ::= return [explist]
  Token keyword = previous();
  std::unique_ptr<Expr> value = nullptr;
  if (!done() && !isBlockCloser(peek().type))
  {
    value = expression();
  }
  // TODO: ADD SUPPORT FOR MULTIPLE RETURN VALUES
  return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::breakStatement()
{
  return std::make_unique<BreakStmt>(previous());
}

std::unique_ptr<Stmt> Parser::expressionStatement()
{
  // stat ::= varlist `=` explist | functioncall
  std::unique_ptr<Expr> expr = expression();
  if (match(TokenType::EQUAL))
  {
    auto* name = dynamic_cast<NameExpr*>(expr.get());
    if (!name) throw ParseError("Invalid assignment target.", peek().line);
    std::unique_ptr<Expr> value = expression();
    return std::make_unique<AssignStmt>(name->name, std::move(value));
  }
  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::blockStatement()
{
  // block ::= chunk
  // chunk ::= {stat [`;`]} [laststat [`;`]]
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!done() && !isBlockCloser(peek().type))
  {
    statements.push_back(statement());
  }

  if (match(TokenType::RETURN)) statements.push_back(returnStatement());
  else if (match(TokenType::BREAK)) statements.push_back(breakStatement());

  if (done()) throw ParseError("Expected 'end' to close block", peek().line);
  return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::localFunctionStatement()
{
  Token name = consume(TokenType::IDENTIFIER, "Expected function name after declaration");
  FuncBody body = parseFuncBody();
  auto func = std::make_unique<FunctionExpr>(std::move(body.params), std::move(body.body));
  return std::make_unique<LocalStmt>(std::move(name), std::move(func));
}

std::unique_ptr<Stmt> Parser::localStatement()
{
  if (match(TokenType::FUNCTION))
  {
    return localFunctionStatement();
  }
  Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'local'");
  std::unique_ptr<Expr> value = nullptr;
  if (match(TokenType::EQUAL))
  {
    value = expression();
  }
  return std::make_unique<LocalStmt>(std::move(name), std::move(value));
}

std::unique_ptr<Stmt> Parser::whileStatement()
{
  // stat ::= while exp do block end
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::DO, "Expected 'do' after condition");
  std::unique_ptr<Stmt> block = blockStatement();
  consume(TokenType::END, "Expected 'end' after block");
  return std::make_unique<WhileStmt>(std::move(condition), std::move(block));
}

std::unique_ptr<Stmt> Parser::repeatStatement()
{
  // stat ::= repeat block until exp
  std::unique_ptr<Stmt> body = blockStatement();
  consume(TokenType::UNTIL, "Expected 'until' after block");
  std::unique_ptr<Expr> condition = expression();
  return std::make_unique<RepeatStmt>(std::move(body), std::move(condition));
}

std::unique_ptr<Stmt> Parser::elseIfStatement()
{
  // {elseif exp then block}
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::THEN, "Expected 'then' after expression");
  std::unique_ptr<Stmt> block = blockStatement();
  return std::make_unique<IfStmt>(std::move(condition), std::move(block), nullptr);
}

std::unique_ptr<Stmt> Parser::ifStatement()
{
  // stat ::= if exp then block {elseif exp then block} [else block] end
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::THEN, "Expected 'then' after expression");
  std::unique_ptr<Stmt> block = blockStatement();

  std::unique_ptr<Stmt> ifStmt =
      std::make_unique<IfStmt>(std::move(condition), std::move(block), nullptr);
  IfStmt* cur = (IfStmt*)ifStmt.get();

  while (match(TokenType::ELSEIF))
  {
    std::unique_ptr<Stmt> elseIf = elseIfStatement();
    IfStmt* next = (IfStmt*)elseIf.get(); // get pointer BEFORE move
    cur->elseBranch = std::move(elseIf);
    cur = next;
  }

  if (match(TokenType::ELSE))
  {
    cur->elseBranch = blockStatement();
  }

  consume(TokenType::END, "Expected 'end' after if statement");
  return ifStmt;
}

std::unique_ptr<Stmt> Parser::doStatement()
{
  std::unique_ptr<Stmt> block = blockStatement();
  consume(TokenType::END, "Expected 'end' after block");
  return std::make_unique<DoStmt>(std::move(block));
}

std::unique_ptr<Expr> Parser::expression()
{
  // exp  ::= (nil | false | true | Number | String | `...` | function | prefixexp |
  //           tableconstructor | unop exp) exp'

  // exp' ::= binop exp exp' | ε

  // ε ::= nothing

  // exp ::= (nil | false | true | Number | String)
  return orExpression();
}

std::unique_ptr<Expr> Parser::expressionTemplate(std::initializer_list<TokenType> tokenTypes,
                                                 std::function<std::unique_ptr<Expr>()> expr)
{
  std::unique_ptr<Expr> left = expr();
  while (match(tokenTypes))
  {
    Token op = previous();
    std::unique_ptr<Expr> right = expr();
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }
  return left;
}

std::unique_ptr<Expr> Parser::expressionTemplate(std::initializer_list<TokenType> tokenTypes,
                                                 std::function<std::unique_ptr<Expr>()> base,
                                                 std::function<std::unique_ptr<Expr>()> self)
{
  std::unique_ptr<Expr> left = base();
  if (match(tokenTypes))
  {
    Token op = previous();
    std::unique_ptr<Expr> right = self();
    return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }
  return left;
}

std::unique_ptr<Expr> Parser::orExpression()
{
  return expressionTemplate({TokenType::OR}, [this]() { return andExpression(); });
}

std::unique_ptr<Expr> Parser::andExpression()
{
  return expressionTemplate({TokenType::AND}, [this]() { return comparisonExpression(); });
}

std::unique_ptr<Expr> Parser::comparisonExpression()
{
  return expressionTemplate({TokenType::LESS,
                             TokenType::GREATER,
                             TokenType::LESS_EQUAL,
                             TokenType::GREATER_EQUAL,
                             TokenType::EQUAL_EQUAL,
                             TokenType::TILDE_EQUAL},
                            [this]() { return concatExpression(); });
}

std::unique_ptr<Expr> Parser::concatExpression()
{
  return expressionTemplate(
      {TokenType::DOT_DOT},
      [this]() { return termExpression(); },
      [this]() { return concatExpression(); });
}

std::unique_ptr<Expr> Parser::termExpression()
{
  return expressionTemplate({TokenType::PLUS, TokenType::MINUS},
                            [this]() { return factorExpression(); });
}

std::unique_ptr<Expr> Parser::factorExpression()
{
  return expressionTemplate({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT},
                            [this]() { return unaryExpression(); });
}

std::unique_ptr<Expr> Parser::powerExpression()
{
  return expressionTemplate(
      {TokenType::CARET},
      [this]() { return baseExpression(); },
      [this]() { return powerExpression(); });
}

// base ::= nil | false | true | Number | String | `...` | function | prefixexp | tableconstructor
// | unop exp
std::unique_ptr<Expr> Parser::baseExpression()
{
  std::unique_ptr<Expr> to_return;
  // exp ::= (nil | false | true | Number | String)
  to_return = literalExpression();
  if (to_return != nullptr) return to_return;
  // exp ::= function funcbody
  if (check(TokenType::FUNCTION))
  {
    match(TokenType::FUNCTION);
    return functionExpression();
  }
  // exp ::= (prefixexp)
  to_return = prefixExpression();
  if (to_return != nullptr) return to_return;
  // maybe throw here, should cover everything in base
  throw ParseError("Expected expression.", peek().line);
}

std::unique_ptr<Expr> Parser::literalExpression()
{
  // nil | false | true | Number | String
  if (match(TokenType::NUMBER)) return std::make_unique<LiteralExpr>(previous().literal);
  if (match(TokenType::STRING)) return std::make_unique<LiteralExpr>(previous().literal);
  if (match(TokenType::NIL)) return std::make_unique<LiteralExpr>(std::monostate{});
  if (match(TokenType::TRUE)) return std::make_unique<LiteralExpr>(true);
  if (match(TokenType::FALSE)) return std::make_unique<LiteralExpr>(false);
  return nullptr;
}

std::unique_ptr<Expr> Parser::unaryExpression()
{
  // unop exp ::= `-` | not | `#`
  if (match(TokenType::MINUS, TokenType::NOT, TokenType::HASH))
  {
    Token op = previous();
    std::unique_ptr<Expr> right = powerExpression();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }
  return powerExpression();
}

inline std::unique_ptr<Expr> Parser::varExpression()
{
  return prefixExpression();
}

std::unique_ptr<Expr> Parser::prefixExpression()
{
  // prefixexp ::= var | functioncall | `(` exp `)`
  // var ::= Name | prefixexp `[` exp `]` | prefixexp `.` Name

  std::unique_ptr<Expr> expr;

  if (match(TokenType::IDENTIFIER))
  {
    expr = std::make_unique<NameExpr>(previous());
  }
  else if (match(TokenType::LEFT_PAREN))
  {
    expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
  }
  else return nullptr;

  for (;;)
  {
    if (match(TokenType::LEFT_PAREN)) // functioncall
    {
      // args ::= `(` [explist] `)`
      std::vector<std::unique_ptr<Expr>> args;
      if (!check(TokenType::RIGHT_PAREN))
      {
        do
        {
          args.push_back(expression());
        } while (match(TokenType::COMMA));
      }
      Token paren = consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
      expr = std::make_unique<CallExpr>(std::move(expr), paren, std::move(args));
    }
    else if (match(TokenType::LEFT_BRACKET)) // t[exp]
    {
      std::unique_ptr<Expr> index = expression();
      consume(TokenType::RIGHT_BRACKET, "Expected ']' after expression.");
      expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
    }
    else if (match(TokenType::DOT)) // t.name
    {
      Token field = consume(TokenType::IDENTIFIER, "Expected field name after '.'.");
      expr = std::make_unique<FieldExpr>(std::move(expr), std::move(field));
    }
    else break;
  }
  return expr;
}

Token Parser::previous() const
{
  return this->tokens[current - 1];
}

Token Parser::advance()
{
  if (!done()) current++;
  return previous();
}

bool Parser::check(TokenType type)
{
  return done() ? false : peek().type == type;
}

Token Parser::consume(TokenType type, std::string message)
{
  if (!check(type)) throw ParseError(message, peek().line);
  return advance();
}

Token Parser::peek()
{
  return tokens[current];
}

bool Parser::done()
{
  return peek().type == TokenType::EOF_LUA;
}

bool Parser::match(std::initializer_list<TokenType> types)
{
  for (TokenType type : types)
  {
    if (!check(type)) continue;
    advance();
    return true;
  }
  return false;
}

void Parser::sync()
{
  advance();
  while (!done())
  {
    switch (peek().type)
    {
      case TokenType::LOCAL:
      case TokenType::FUNCTION:
      case TokenType::IF:
      case TokenType::WHILE:
      case TokenType::FOR:
      case TokenType::AND:
      case TokenType::OR:
      case TokenType::REPEAT:
      case TokenType::DO:
      case TokenType::RETURN:
      case TokenType::BREAK:
      case TokenType::END:
      case TokenType::ELSE:
      case TokenType::ELSEIF:
      case TokenType::UNTIL: return;
      default: break;
    }
    advance();
  }
}
