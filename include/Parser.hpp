#pragma once
#include "expressions/BinaryExpr.hpp"
#include "expressions/Expr.hpp"
#include "statements/IfStmt.hpp"
#include "statements/Stmt.hpp"
#include "tokens/Token.hpp"
#include <functional>
#include <memory>
#include <vector>

class ParseError : public std::runtime_error
{
public:
  int line;
  ParseError(const std::string& message, int line) : std::runtime_error(message), line(line) {}
};

struct FuncBody
{
  std::vector<Token> params;
  std::unique_ptr<Stmt> body;
};

/*
chunk ::= {stat [`;´]} [laststat [`;´]]

  block ::= chunk

  stat ::=  varlist `=´ explist |
     functioncall |
     do block end |
     while exp do block end |
     repeat block until exp |
     if exp then block {elseif exp then block} [else block] end |
     for Name `=´ exp `,´ exp [`,´ exp] do block end |
     for namelist in explist do block end |
     function funcname funcbody |
     local function Name funcbody |
     local namelist [`=´ explist]

  laststat ::= return [explist] | break

  funcname ::= Name {`.´ Name} [`:´ Name]

  varlist ::= var {`,´ var}

  var ::=  Name | prefixexp `[´ exp `]´ | prefixexp `.´ Name

  namelist ::= Name {`,´ Name}

  explist ::= {exp `,´} exp

  exp ::=  nil | false | true | Number | String | `...´ | function |
     prefixexp | tableconstructor | exp binop exp | unop exp

  prefixexp ::= var | functioncall | `(´ exp `)´

  functioncall ::=  prefixexp args | prefixexp `:´ Name args

  args ::=  `(´ [explist] `)´ | tableconstructor | String

  function ::= function funcbody

  funcbody ::= `(´ [parlist] `)´ block end

  parlist ::= namelist [`,´ `...´] | `...´

  tableconstructor ::= `{´ [fieldlist] `}´

  fieldlist ::= field {fieldsep field} [fieldsep]

  field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp

  fieldsep ::= `,´ | `;´

  binop ::= `+´ | `-´ | `*´ | `/´ | `^´ | `%´ | `..´ |
     `<´ | `<=´ | `>´ | `>=´ | `==´ | `~=´ |
     and | or

  unop ::= `-´ | not | `#´
*/
class Parser
{
public:
  Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}
  std::vector<std::unique_ptr<Stmt>> parse();

private:
  std::vector<Token> tokens;
  int current = 0;

private:
  bool done();
  Token peek();
  bool check(TokenType type);
  Token advance();
  Token previous() const;
  Token consume(TokenType type, std::string message);
  bool match(std::initializer_list<TokenType> types);
  std::unique_ptr<Stmt> statement();

  // functions
  FuncBody parseFuncBody();
  std::unique_ptr<Stmt> functionStatement();
  std::unique_ptr<Stmt> localFunctionStatement();
  std::unique_ptr<Expr> functionExpression();

  /* TODO: finish */
  std::unique_ptr<Stmt> forStatement();
  std::unique_ptr<Stmt> forRangeStatement(Token);
  std::unique_ptr<Stmt> forEachStatement(Token);
  std::unique_ptr<Expr> callExpression();

  /********************************************/

  std::unique_ptr<Stmt> returnStatement();
  std::unique_ptr<Stmt> breakStatement();
  std::unique_ptr<Stmt> expressionStatement();
  std::unique_ptr<Stmt> assignStatement();
  std::unique_ptr<Stmt> localStatement();
  std::unique_ptr<Stmt> whileStatement();
  std::unique_ptr<Stmt> repeatStatement();
  std::unique_ptr<Stmt> ifStatement();
  std::unique_ptr<Stmt> elseIfStatement();
  std::unique_ptr<Stmt> doStatement();
  std::unique_ptr<Stmt> blockStatement();
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> expressionPrime();

  // exact order
  std::unique_ptr<Expr> orExpression();
  std::unique_ptr<Expr> andExpression();
  std::unique_ptr<Expr> comparisonExpression();
  std::unique_ptr<Expr> concatExpression();
  std::unique_ptr<Expr> termExpression();
  std::unique_ptr<Expr> factorExpression();
  std::unique_ptr<Expr> unaryExpression();
  std::unique_ptr<Expr> powerExpression();
  std::unique_ptr<Expr> baseExpression();
  std::unique_ptr<Expr> literalExpression();
  std::unique_ptr<Expr> varExpression();
  std::unique_ptr<Expr> prefixExpression();
  std::vector<std::unique_ptr<Stmt>> block();
  void sync();

  std::unique_ptr<Expr> expressionTemplate(std::initializer_list<TokenType>,
                                           std::function<std::unique_ptr<Expr>()> expr);
  std::unique_ptr<Expr> expressionTemplate(std::initializer_list<TokenType> tokenTypes,
                                           std::function<std::unique_ptr<Expr>()> base,
                                           std::function<std::unique_ptr<Expr>()> self);

  template <typename... TokenTypes> bool match(TokenTypes... tokenTypes)
  {
    return match({tokenTypes...});
  }
};
