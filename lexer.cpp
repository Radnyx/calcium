#include "lexer.h"
#include <cassert>

bool Lexer::eof() const {
    return index >= program.size();
}

char Lexer::get() const {
    assert(!eof());
    return program[index];
}

void Lexer::advance() {
    index++;
    column++;
}

void Lexer::skipWhitespace() {
    char c;
    while(!eof() && iswspace(c = get())) {
        advance();

        if (c == '\n') {
            line++;
            column = 1;
        }
    }
}

bool Lexer::parseIdentifier(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;
    if (!isalpha(get())) return false;

    token->type = IDENTIFIER;
    token->startIndex = index;

    advance();
    while (!eof() && isalnum(get())) advance();

    token->endIndex = index;
    return true;
}

bool Lexer::parseExact(Token * const token, TokenType type, const std::string & str) {
    int startIndex = index;

    token->type = type;
    token->startIndex = startIndex;

    for (char c : str) {
        if (eof() || c != get()) {
            index = startIndex;
            return false;
        }

        advance();
    }

    token->endIndex = index;

    return true;
}

Error Lexer::tokenize(std::vector<Token> & tokens) {
    skipWhitespace();

    while (!eof()) {
        Token token;

        bool success = 
            parseExact(&token, UNIT, "unit") || 
            parseExact(&token, OPEN_PAREN, "(") || 
            parseExact(&token, CLOSE_PAREN, ")") || 
            parseExact(&token, OPEN_BRACE, "{") || 
            parseExact(&token, CLOSE_BRACE, "}") || 
            parseIdentifier(&token);

        if (!success) {
            std::cerr << "ERR: invalid token at line " << line << ", column " << column << std::endl;
            return INVALID_TOKEN_ERR;
        }

        tokens.push_back(token);

        skipWhitespace();
    }

    return NO_ERR;
}
