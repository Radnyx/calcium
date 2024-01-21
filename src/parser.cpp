#include "../include/parser.h"
#include <cassert>

Parser::Parser(const std::vector<Token> & tokens) : tokens(tokens) {
    index = 0;
}

Error Parser::parse(std::vector<std::unique_ptr<AST>> & ast) {
    while (!eof()) {
        auto prototype = parseFunctionPrototype();
        if (prototype == nullptr) {
            std::cerr << "ERR: invalid top level statement" << std::endl;
            return ERR_INVALID_TOP_LEVEL_STATEMENT;
        }

        if (eof()) {
            std::cerr << "ERR: incomplete function prototype" << std::endl;
            return ERR_INCOMPLETE_FUNCTION_PROTOTYPE;
        }

        if (get().type == SEMICOLON) {
            index++;
            
            ast.push_back(std::make_unique<FunctionDeclarationAST>(prototype));
        } else {
            auto body = parseBody();
            if (body == nullptr) {
                std::cerr << "ERR: function definition missing body" << std::endl;
                return ERR_INCOMPLETE_FUNCTION_PROTOTYPE;
            }

            ast.push_back(std::make_unique<FunctionDefinitionAST>(prototype, body));
        }
    }

    return ERR_NONE;
}

std::unique_ptr<BodyAST> Parser::parseBody() {
    int startIndex = index;

    if (!expect(OPEN_BRACE)) {
        index = startIndex;
        return nullptr;
    }

    auto statements = parseStatementList();

    if (!expect(CLOSE_BRACE)) {
        index = startIndex;
        return nullptr;
    }

    return std::make_unique<BodyAST>(statements);
}

std::vector<std::unique_ptr<AST>> Parser::parseStatementList() {
    std::vector<std::unique_ptr<AST>> statements;

    auto statement = parseStatement();
    while (statement != nullptr) {
        statements.push_back(std::move(statement));
        statement = parseStatement();
    }

    return statements;
}

std::unique_ptr<AST> Parser::parseStatement() {
    int startIndex = index;

    auto statement = parseExpression();
    if (statement == nullptr) {
        return nullptr;
    }

    if (!expect(SEMICOLON)) {
        index = startIndex;
        return nullptr;
    }

    return statement;
}

std::unique_ptr<ExpressionAST> Parser::parseExpression() {
    std::unique_ptr<ExpressionAST> expr = parseStringLiteral();
    if (expr) return expr;

    return parseFunctionCall();
}

std::unique_ptr<StringLiteralAST> Parser::parseStringLiteral() {
    if (eof()) return nullptr;

    Token tok = get();
    if (tok.type == STRING_LITERAL) {
        index++;
        return std::make_unique<StringLiteralAST>(tok);
    }
    return nullptr;
}

std::unique_ptr<FunctionCallAST> Parser::parseFunctionCall() {
    int startIndex = index;

    Token name;
    if (!(expectIdentifier(&name) && expect(OPEN_PAREN))) {
        index = startIndex;
        return nullptr;
    }

    auto expressions = parseExpressionList();

    if (!expect(CLOSE_PAREN)) {
        index = startIndex;
        return nullptr;
    }

    return std::make_unique<FunctionCallAST>(name, expressions);
}

std::vector<std::unique_ptr<ExpressionAST>> Parser::parseExpressionList() {
    std::vector<std::unique_ptr<ExpressionAST>> expressions;

    auto expression = parseExpression();
    while (expression != nullptr) {
        expressions.push_back(std::move(expression));

        int lastIndex = index;
        if (!expect(COMMA)) {
            index = lastIndex;
            break;
        }

        expression = parseExpression();
        // TODO: ALLOWS TRAILING COMMA!
    }

    return expressions;
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


std::unique_ptr<TypeAST> Parser::parseType() {
    if (eof()) return nullptr;

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

bool Parser::parseParameter(Token * name, std::unique_ptr<TypeAST> * type) {
    assert(name != nullptr);
    assert(type != nullptr);
    int startIndex = index;

    if (expectIdentifier(name) && expect(COLON) && expectType(type)) {
        return true;
    }

    index = startIndex;
    if (expectType(type)) return true;

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
        // TODO: ALLOWS TRAILING COMMA!
    }

    return parameters;
}


bool Parser::expect(TokenType tokenType) {
    if (eof()) return false;

    auto tok = get();
    if (tok.type != tokenType) return false;
    index++;
    return true;
}

bool Parser::expectIdentifier(Token * token) {
    assert(token != nullptr);
    if (eof()) return false;

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