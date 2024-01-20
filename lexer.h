#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <vector>
#include "errors.h"

enum TokenType {
    UNIT,
    IDENTIFIER,
    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE
};

struct Token {
    TokenType type;
    int startIndex;
    int endIndex;
};

class Lexer {
public:
    Lexer(const std::string & program) : program(program) {
        index = 0;
        line = 1;
        column = 1;
    }

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
    bool parseIdentifier(Token * const token);
    bool parseExact(Token * const token, TokenType type, const std::string & str);
};

#endif // LEXER_H
