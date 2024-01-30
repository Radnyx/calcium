#include "../include/lexer.h"
#include <cassert>

static std::string takeExample(const std::string & text, size_t index) {
    size_t exampleLength = std::min(20ull, text.size() - index);
    auto example = text.substr(index, exampleLength);
    auto newLineIndex = example.find("\n");
    if (newLineIndex != std::string::npos) {
        example = example.substr(0, newLineIndex);
    }
    return example;
}

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

bool Lexer::readFloatLiteral(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;

    size_t startIndex = index;

    char c;
    while (!eof() && isdigit(c = get())) {
        advance();
    }

    if (c != '.') {
        rewind(startIndex);
        return false;
    }

    advance();
    while (!eof() && isdigit(c = get())) {
        advance();
    }

    // check if no digits
    if (index == startIndex + 1) {
        rewind(startIndex);
        return false;
    }

    token->type = FLOAT_LITERAL;
    token->startIndex = startIndex;
    token->line = line;
    token->column = column;
    token->endIndex = index;
    
    return true;
}

bool Lexer::readIntLiteral(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;
    if (!isdigit(get())) return false;

    char c;
    while (!eof() && isdigit(c = get())) {
        advance();
    }

    token->type = INT_LITERAL;
    token->startIndex = index;
    token->line = line;
    token->column = column;
    token->endIndex = index;
    
    return true;
}

bool Lexer::readStringLiteral(Token * const token) {
    assert(token != nullptr);

    if (eof()) return false;
    if (get() != '"') return false;

    size_t startIndex = index;
    advance();

    if (eof()) {
        rewind(startIndex);
        return false;
    }

    char c;
    while ((c = get()) != '"') {
        if (c == '\n') {
            rewind(startIndex);
            return false;
        }
        advance();

        if (eof()) {
            rewind(startIndex);
            return false;
        }
    }
    
    advance(); // '"'

    token->type = STRING_LITERAL;
    token->startIndex = index;
    token->line = line;
    token->column = column;
    token->endIndex = index;

    return true;
}

bool Lexer::readExact(Token * const token, TokenType type, const std::string & str) {
    assert(token != nullptr);
    
    size_t startIndex = index;

    for (char c : str) {
        if (eof() || c != get()) {
            rewind(startIndex);
            return false;
        }

        advance();
    }

    token->type = type;
    token->startIndex = startIndex;
    token->line = line;
    token->column = column;
    token->endIndex = index;

    return true;
}

void Lexer::rewind(size_t oldIndex) {
    assert(oldIndex <= index);

    // TODO: doesn't support "tokens" across multiple lines
    size_t difference = index - oldIndex;
    column -= difference;

    index = oldIndex;
}

Error Lexer::tokenize(std::vector<Token> & tokens) {
    skipWhitespace();

    while (!eof()) {
        Token token;

        bool success = 
            readExact(&token, FUN, "fun") || 
            readExact(&token, KER, "ker") || 
            readExact(&token, KERNEL, "Kernel") || 
            readExact(&token, STRUCT, "struct") || 
            readExact(&token, UNIT, "unit") || 
            readExact(&token, INT, "int") || 
            readExact(&token, BYTE, "byte") || 
            readExact(&token, VEC2, "vec2") || 
            readExact(&token, VEC4, "vec4") || 
            readExact(&token, LET, "let") || 
            readExact(&token, WHILE, "while") || 
            readExact(&token, RETURN, "return") || 
            readExact(&token, STAR, "*") || 
            readExact(&token, OPEN_PAREN, "(") || 
            readExact(&token, CLOSE_PAREN, ")") || 
            readExact(&token, OPEN_BRACE, "{") || 
            readExact(&token, CLOSE_BRACE, "}") || 
            readExact(&token, COMMA, ",") || 
            readExact(&token, COLON, ":") || 
            readExact(&token, SEMICOLON, ";") || 
            readExact(&token, EQUALS, "=") || 
            readExact(&token, NOT, "!") || 
            readExact(&token, MINUS, "-") || 
            readIdentifier(&token) || 
            readFloatLiteral(&token) ||
            readIntLiteral(&token) ||
            readStringLiteral(&token);

        if (!success) {
            auto example = takeExample(program, index);
            std::cerr << "ERR: invalid token at line " << line << 
                ", column " << column << ": \"" << example << "\"" << std::endl;
            return ERR_INVALID_TOKEN;
        }

        tokens.push_back(token);

        skipWhitespace();
    }

    return ERR_NONE;
}
