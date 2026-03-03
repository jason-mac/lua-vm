#pragma once

enum class TokenType {

  LEFT_PAREN,    // (
  RIGHT_PAREN,   // )
  LEFT_BRACE,    // {
  RIGHT_BRACE,   // }
  LEFT_BRACKET,  // [
  RIGHT_BRACKET, // ]
  COMMA,         // ,
  DOT,           // .
  COLON,         // :
  SEMICOLON,     // ;
  PLUS,          // +
  MINUS,         // -
  STAR,          // *
  SLASH,         // /
  PERCENT,       // %
  CARET,         // ^
  HASH,          // #

  // One or two character tokens
  EQUAL,         // =
  EQUAL_EQUAL,   // ==
  TILDE_EQUAL,   // ~=
  LESS,          // <
  LESS_EQUAL,    // <=
  GREATER,       // >
  GREATER_EQUAL, // >=
  DOT_DOT,       // ..
  DOT_DOT_DOT,   // ...

  // Literals
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords
  AND,
  BREAK,
  DO,
  ELSE,
  ELSEIF,
  END,
  FALSE,
  FOR,
  FUNCTION,
  IF,
  IN,
  LOCAL,
  NIL,
  NOT,
  OR,
  REPEAT,
  RETURN,
  THEN,
  TRUE,
  UNTIL,
  WHILE,

  EOF_LUA,
};
