#include "../include/lexer.h"
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

    skipComment();
}

void Lexer::skipComment() {
    if (index < program.size() - 2 && get() == '/' && program[index + 1] == '/') {
        while (!eof() && get() != '\n') advance();
        index++;
        line++;
        column = 1;

        skipWhitespace();
    }
}

bool Lexer::readIdentifier(Token * const token) {
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

bool Lexer::readStringLiteral(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;
    if (get() != '"') return false;

    int startIndex = index;
    advance();

    token->type = STRING_LITERAL;
    token->startIndex = index;

    char c;
    while ((c = get()) != '"') {
        if (c == '\n') {
            index = startIndex;
            return false;
        }
        advance();
    }

    token->endIndex = index;
    advance();

    return true;
}

bool Lexer::readExact(Token * const token, TokenType type, const std::string & str) {
    assert(token != nullptr);
    
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
            readExact(&token, UNIT, "unit") || 
            readExact(&token, OPEN_PAREN, "(") || 
            readExact(&token, CLOSE_PAREN, ")") || 
            readExact(&token, OPEN_BRACE, "{") || 
            readExact(&token, CLOSE_BRACE, "}") || 
            readExact(&token, STAR, "*") || 
            readExact(&token, BYTE, "byte") || 
            readExact(&token, COLON, ":") || 
            readExact(&token, SEMICOLON, ";") || 
            readExact(&token, EQUALS, "=") || 
            readIdentifier(&token) || 
            readStringLiteral(&token);

        if (!success) {
            std::cerr << "ERR: invalid token at line " << line << ", column " << column << std::endl;
            return INVALID_TOKEN_ERR;
        }

        tokens.push_back(token);

        skipWhitespace();
    }

    return NO_ERR;
}
