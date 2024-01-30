#include "../include/lexer.h"
#include <cassert>

Token::Token() : type(NULL_TOKEN) {}

Lexer::Lexer(const std::string & program) : program(program) {
    index = 0;
    line = 1;
    column = 1;
}

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
    token->line = line;
    token->column = column;

    advance();
    while (!eof() && isalnum(get())) advance();

    token->endIndex = index;
    return true;
}

bool Lexer::readStringLiteral(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;
    if (get() != '"') return false;

    size_t startIndex = index;
    advance();

    token->type = STRING_LITERAL;
    token->startIndex = index;
    token->line = line;
    token->column = column;

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
    
    size_t startIndex = index;

    token->type = type;
    token->startIndex = startIndex;
    token->line = line;
    token->column = column;

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
            readExact(&token, FUN, "fun") || 
            readExact(&token, UNIT, "unit") || 
            readExact(&token, INT, "int") || 
            readExact(&token, BYTE, "byte") || 
            readExact(&token, STAR, "*") || 
            readExact(&token, OPEN_PAREN, "(") || 
            readExact(&token, CLOSE_PAREN, ")") || 
            readExact(&token, OPEN_BRACE, "{") || 
            readExact(&token, CLOSE_BRACE, "}") || 
            readExact(&token, COMMA, ",") || 
            readExact(&token, COLON, ":") || 
            readExact(&token, SEMICOLON, ";") || 
            readExact(&token, EQUALS, "=") || 
            readIdentifier(&token) || 
            readStringLiteral(&token);

        if (!success) {
            std::cerr << "ERR: invalid token at line " << line << ", column " << column << std::endl;
            return ERR_INVALID_TOKEN;
        }

        tokens.push_back(token);

        skipWhitespace();
    }

    return ERR_NONE;
}
