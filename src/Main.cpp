#include "Lexer.hpp"
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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

static std::string literalToString(const Literal &lit) {
    if (std::holds_alternative<std::monostate>(lit)) return "nil";
    if (std::holds_alternative<double>(lit)) return std::to_string(std::get<double>(lit));
    if (std::holds_alternative<std::string>(lit)) return "\"" + std::get<std::string>(lit) + "\"";
    if (std::holds_alternative<bool>(lit)) return std::get<bool>(lit) ? "true" : "false";
    return "unknown";
}

static void printExpr(const Expr *expr, int indent) {
    std::string pad(indent * 2, ' ');
    if (auto *e = dynamic_cast<const LiteralExpr *>(expr)) {
        std::cout << pad << "LiteralExpr(" << literalToString(e->value) << ")\n";
    } else if (auto *e = dynamic_cast<const NameExpr *>(expr)) {
        std::cout << pad << "NameExpr(" << e->name.lexeme << ")\n";
    } else if (auto *e = dynamic_cast<const BinaryExpr *>(expr)) {
        std::cout << pad << "BinaryExpr(" << e->op.lexeme << ")\n";
        printExpr(e->left.get(), indent + 1);
        printExpr(e->right.get(), indent + 1);
    } else if (auto *e = dynamic_cast<const UnaryExpr *>(expr)) {
        std::cout << pad << "UnaryExpr(" << e->operator_.lexeme << ")\n";
        printExpr(e->rightOperand.get(), indent + 1);
    } else {
        std::cout << pad << "UnknownExpr\n";
    }
}

static void printStmt(const Stmt *stmt, int indent) {
    std::string pad(indent * 2, ' ');
    if (auto *s = dynamic_cast<const LocalStmt *>(stmt)) {
        std::cout << pad << "LocalStmt(name=" << s->name.lexeme << ")\n";
        if (s->initializer) {
            printExpr(s->initializer.get(), indent + 1);
        } else {
            std::cout << pad << "  (no initializer)\n";
        }
    } else if (auto *s = dynamic_cast<const AssignStmt *>(stmt)) {
        std::cout << pad << "AssignStmt(name=" << s->name.lexeme << ")\n";
        printExpr(s->value.get(), indent + 1);
    } else if (auto *s = dynamic_cast<const ExpressionStmt *>(stmt)) {
        std::cout << pad << "ExpressionStmt\n";
        printExpr(s->expression.get(), indent + 1);
    } else if (auto *s = dynamic_cast<const ReturnStmt *>(stmt)) {
        std::cout << pad << "ReturnStmt\n";
        if (s->value) printExpr(s->value.get(), indent + 1);
    } else if (auto *s = dynamic_cast<const BlockStmt *>(stmt)) {
        std::cout << pad << "BlockStmt\n";
        for (const auto &inner : s->statements) {
            printStmt(inner.get(), indent + 1);
        }
    } else if (auto *s = dynamic_cast<const IfStmt *>(stmt)) {
        std::cout << pad << "IfStmt\n";
        std::cout << pad << "  condition:\n";
        printExpr(s->condition.get(), indent + 2);
        std::cout << pad << "  then:\n";
        if (s->thenBranch) printStmt(s->thenBranch.get(), indent + 2);
        if (s->elseBranch) {
            std::cout << pad << "  else:\n";
            printStmt(s->elseBranch.get(), indent + 2);
        }
    } else if (auto *s = dynamic_cast<const WhileStmt *>(stmt)) {
        std::cout << pad << "WhileStmt\n";
        std::cout << pad << "  condition:\n";
        printExpr(s->condition.get(), indent + 2);
        std::cout << pad << "  body:\n";
        if (s->body) printStmt(s->body.get(), indent + 2);
    } else {
        std::cout << pad << "UnknownStmt\n";
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./luajvm <file.lua>\n";
        return EXIT_FAILURE;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return EXIT_FAILURE;
    }
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    auto stmts = parser.parse();
    for (const auto &stmt : stmts) {
        printStmt(stmt.get(), 0);
    }
    return EXIT_SUCCESS;
}
