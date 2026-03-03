#pragma once

#include "Common.hpp"
#include "TokenType.hpp"
#include <string>

class Token {
public:
  Token(TokenType type, std::string lexeme, Literal literal, int line)
      : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)),
        line(line) {}

  TokenType type;
  std::string lexeme;
  Literal literal;
  int line;
};
