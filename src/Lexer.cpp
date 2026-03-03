#include "Lexer.hpp"
#include <stdexcept>
#include <string>
#include <vector>

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"and", TokenType::AND},
    {"break", TokenType::BREAK},
    {"do", TokenType::DO},
    {"else", TokenType::ELSE},
    {"elseif", TokenType::ELSEIF},
    {"end", TokenType::END},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"function", TokenType::FUNCTION},
    {"if", TokenType::IF},
    {"in", TokenType::IN},
    {"local", TokenType::LOCAL},
    {"nil", TokenType::NIL},
    {"not", TokenType::NOT},
    {"or", TokenType::OR},
    {"repeat", TokenType::REPEAT},
    {"return", TokenType::RETURN},
    {"then", TokenType::THEN},
    {"true", TokenType::TRUE},
    {"until", TokenType::UNTIL},
    {"while", TokenType::WHILE},
};

bool Lexer::isAtEnd() { return this->current >= this->source.size(); }

void Lexer::scanToken() {
  char c = advance();

  switch (c) {
  case '(':
    addToken(TokenType::LEFT_PAREN);
    break;
  case ')':
    addToken(TokenType::RIGHT_PAREN);
    break;
  case '{':
    addToken(TokenType::LEFT_BRACE);
    break;
  case '}':
    addToken(TokenType::RIGHT_BRACE);
    break;
  case ',':
    addToken(TokenType::COMMA);
    break;
  case '[': {
    if (!match('[')) {
      addToken(TokenType::LEFT_BRACKET);
      break;
    }
    size_t string_begin = current;
    while (!isAtEnd() && !(peek() == ']' && peekNext() == ']')) {
      if (peek() == '\n') {
        line++;
      }
      advance();
    }
    if (isAtEnd()) {
      throw std::runtime_error("message");
    }
    std::string str = source.substr(string_begin, current - string_begin);
    advance();
    advance();
    addToken(TokenType::STRING, str);
    break;
  }
  case ']':
    addToken(TokenType::RIGHT_BRACKET);
    break;
  case ':':
    addToken(TokenType::COLON);
    break;
  case '.':
    if (!match('.')) {
      addToken(TokenType::DOT);
      break;
    }
    if (!match('.')) {
      addToken(TokenType::DOT_DOT);
      break;
    }
    addToken(TokenType::DOT_DOT_DOT);
    break;
  case '-':
    // minus -
    if (!match('-')) {
      addToken(TokenType::MINUS);
      break;
    }

    // block comment --[[ ]]
    if (match('[') && match('[')) {
      while (!isAtEnd() && !(peek() == ']' && peekNext() == ']')) {
        if (peek() == '\n') {
          line++;
        }
        advance();
      }
      if (isAtEnd()) {
        throw std::runtime_error("Unterminated block comment");
      }
      advance();
      advance();
      break;
    }

    // comment --
    while (!isAtEnd() && peek() != '\n') {
      advance();
    }
    break;
  case '+':
    addToken(TokenType::PLUS);
    break;
  case ';':
    addToken(TokenType::SEMICOLON);
    break;
  case '*':
    addToken(TokenType::STAR);
    break;
  case '~':
    if (match('=')) {
      addToken(TokenType::TILDE_EQUAL);
    } else {
      throw std::runtime_error("Unexpected '~'");
    }
    break;
  case '=':
    addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    break;
  case '<':
    addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    break;
  case '>':
    addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    break;
  case '/':
    addToken(TokenType::SLASH);
    break;
  case '%':
    addToken(TokenType::PERCENT);
    break;
  case '^':
    addToken(TokenType::CARET);
    break;
  case '#':
    addToken(TokenType::HASH);
    break;
  case ' ':
  case '\t':
  case '\r':
    break;
  case '\n':
    line++;
    break;
  case '"':
  case '\'':
    addString(c);
    break;
  default:
    if (isDigit(c)) {
      addNumber();
    } else if (isAlpha(c)) {
      addIdentifier();
    } else {
      throw std::runtime_error("Unexpected Error at line " +
                               std::to_string(line));
    }
    break;
  }
  return;
}

void Lexer::addIdentifier() {
  while (isAlphaNumeric(peek())) {
    advance();
  }
  std::string text = source.substr(start, current - start);
  try {
    TokenType type = keywords.at(text);
    addToken(type);
  } catch (std::out_of_range) {
    addToken(TokenType::IDENTIFIER);
  }
}

bool Lexer::isAlpha(Byte c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(Byte c) { return '0' <= c && c <= '9'; }

bool Lexer::isAlphaNumeric(Byte c) { return isDigit(c) || isAlpha(c); }

void Lexer::addNumber() {
  while (isDigit(peek())) {
    advance();
  }

  if (peek() == '.' && isDigit(peekNext())) {
    advance();
    while (isDigit(peek())) {
      advance();
    }
  }
  addToken(TokenType::NUMBER, std::stod(source.substr(start, current - start)));
}
void Lexer::addString(char quote) {
  while (peek() != quote && !isAtEnd()) {
    if (peek() == '\n') {
      line++;
    }
    advance();
  }

  if (isAtEnd()) {
    throw std::runtime_error("Unexpected end of input");
    return;
  }

  advance();
  std::string value = source.substr(start + 1, current - start - 2);
  addToken(TokenType::STRING, value);
}

char Lexer::peek() { return this->isAtEnd() ? '\0' : this->source[current]; }

char Lexer::peekNext() {
  return this->current + 1 >= source.size() ? '\0' : source[this->current + 1];
}

bool Lexer::match(char expected) {
  if (isAtEnd() || this->source[current] != expected) {
    return false;
  }
  current++;
  return true;
}

char Lexer::advance() { return this->source[this->current++]; }

void Lexer::addToken(TokenType type, Literal literal) {
  tokens.emplace_back(type, source.substr(start, current - start),
                      std::move(literal), line);
}
void Lexer::addToken(TokenType type) { addToken(type, std::monostate{}); }

std::vector<Token> Lexer::scanTokens() {
  while (!isAtEnd()) {
    start = current;
    scanToken();
  }
  tokens.emplace_back(TokenType::EOF_LUA, "", std::monostate{}, line);
  return tokens;
}
