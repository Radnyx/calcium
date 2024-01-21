#include "../include/parser.h"
#include <cassert>

Parser::Parser(const std::vector<Token> & tokens) : tokens(tokens) {
    index = 0;
}

Error Parser::parse(std::unique_ptr<AST> & ast) {
    auto type = parseFunctionPrototype();

    return NO_ERR;
}

std::unique_ptr<FunctionPrototypeAST> Parser::parseFunctionPrototype() {
    int startIndex = index;

    Token name;
    std::unique_ptr<TypeAST> returnType;

    if (!(expect(FUN) && expectIdentifier(&name) && expect(OPEN_PAREN))) {
        index = startIndex;
        return nullptr;
    }

    auto parameterList = parseParameterList();

    if (!(expect(CLOSE_PAREN) && expect(COLON) && expectType(&returnType))) {
        index = startIndex;
        return nullptr;
    }

    return std::make_unique<FunctionPrototypeAST>(name, parameterList, returnType);
}

bool Parser::parseParameter(Token * name, std::unique_ptr<TypeAST> * type) {
    assert(name != nullptr);
    assert(type != nullptr);
    int startIndex = index;

    if (expectIdentifier(name) && expect(COLON) && expectType(type)) {
        return true;
    }

    index = startIndex;
    if (expectType(type))  return true;

    index = startIndex;
    return false;
}

std::vector<Parameter> Parser::parseParameterList() {
    std::vector<Parameter> parameters;

    Token name;
    std::unique_ptr<TypeAST> type;
    bool hasParameter = parseParameter(&name, &type);
    if (!hasParameter) {
        return parameters;
    }

    while (hasParameter) {
        parameters.push_back({ name, std::move(type) });

        int lastIndex = index;
        if (!expect(COMMA)) {
            index = lastIndex;
            break;
        }

        hasParameter = parseParameter(&name, &type);
    }

    return parameters;
}

std::unique_ptr<TypeAST> Parser::parseType() {
    auto tok = get();
    switch (tok.type) {
    case UNIT:
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_UNIT);
    case BYTE: 
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_BYTE);
    case INT: 
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_INT);
    case STAR:
    {
        index++;
        auto type = parseType();
        if (type) {
            return std::make_unique<PointerTypeAST>(type);
        } else {
            index--;
        }
    }
    default:
        return nullptr;
    }
}

bool Parser::expect(TokenType tokenType) {
    auto tok = get();
    if (tok.type != tokenType) return false;
    index++;
    return true;
}

bool Parser::expectIdentifier(Token * token) {
    assert(token != nullptr);
    auto tok = get();
    if (tok.type != IDENTIFIER) return false;
    *token = tok;
    index++;
    return true;
}

bool Parser::expectType(std::unique_ptr<TypeAST> * type) {
    assert(type != nullptr);
    *type = std::move(parseType());
    return *type != nullptr;
}

bool Parser::eof() const {
    return index >= tokens.size();
}

Token Parser::get() const {
    assert(!eof());
    return tokens[index];
}