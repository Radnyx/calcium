#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <vector>
#include "errors.h"

enum TokenType {
    NULL_TOKEN,
    UNIT,
    INT,
    BYTE,
    STAR,
    COLON,
    FUN,
    IDENTIFIER,
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    COMMA,
    SEMICOLON,
    EQUALS,
    STRING_LITERAL
};

struct Token {
    Token();
    TokenType type;
    int startIndex;
    int endIndex;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string & program);

    Error tokenize(std::vector<Token> & tokens);

private:
    const std::string & program;
    int index;
    int line;
    int column;

    bool eof() const;
    char get() const;

    void advance();
    void skipWhitespace();
    void skipComment();
    bool readIdentifier(Token * const token);
    bool readStringLiteral(Token * const Token);
    bool readExact(Token * const token, TokenType type, const std::string & str);
};

#endif // LEXER_H