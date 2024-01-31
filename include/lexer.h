#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <vector>
#include "Errors.h"

// TODO: add eof token, should relieve some "eof()" checks in the parser
enum TokenType {
    TOK_NULL,
    TOK_UNIT,
    TOK_INT,
    TOK_BYTE,
    TOK_BOOL,
    TOK_STAR,
    TOK_COLON,
    TOK_FUN,
    TOK_STRUCT,
    TOK_KER,
    // KERNEL,
    // VEC2,
    // VEC4,
    TOK_LET,
    TOK_WHILE,
    TOK_RETURN,
    TOK_IDENTIFIER,
    TOK_OPEN_PAREN,
    TOK_CLOSE_PAREN,
    TOK_OPEN_BRACE,
    TOK_CLOSE_BRACE,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_EQUALS,
    TOK_NOT,
    TOK_MINUS,
    TOK_FLOAT_LITERAL,
    TOK_INT_LITERAL,
    TOK_STRING_LITERAL
};

struct Token {
    Token();
    TokenType type;
    size_t startIndex;
    size_t endIndex;
    size_t line;
    size_t column;
};

class Lexer {
public:
    Lexer(const std::string & program);

    Error tokenize(std::vector<Token> & tokens);

private:
    const std::string & program;
    size_t index;
    size_t line;
    size_t column;

    bool eof() const;
    char get() const;

    void advance();
    void skipWhitespace();
    void skipComment();
    bool readIdentifier(Token * const token);
    bool readFloatLiteral(Token * const token);
    bool readIntLiteral(Token * const token);
    bool readStringLiteral(Token * const Token);
    bool readExact(Token * const token, TokenType type, const std::string & str);

    void rewind(size_t oldIndex);
};

#endif // LEXER_H
