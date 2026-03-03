#pragma once

#include "Common.hpp"
#include "Token.hpp"
#include "TokenType.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class Lexer {

public:
  Lexer(std::string source) : source(source), start(0), current(0), line(1) {}
  std::vector<Token> scanTokens();

private:
  static const std::unordered_map<std::string, TokenType> keywords;
  std::vector<Token> tokens;
  std::string source;
  uint32_t start;
  uint32_t current;
  uint32_t line;

private:
  bool isAtEnd();
  void scanToken();
  void addIdentifier();
  bool isAlpha(Byte c);
  bool isDigit(Byte c);
  bool isAlphaNumeric(Byte c);
  void addNumber();
  void addString(char quote);
  char peek();
  char peekNext();
  bool match(char expected);
  char advance();
  void addToken(TokenType type, Literal literal);
  void addToken(TokenType type);
};
