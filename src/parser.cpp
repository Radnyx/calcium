#include "../include/parser.h"
#include <cassert>

bool ParserError::empty() {
    return message.tellp() == std::streampos(0);
}

Parser::Parser(const Program & program, const std::vector<Token> & tokens) 
: program(program), tokens(tokens) {
    index = 0;
}

Error Parser::parse(std::vector<std::unique_ptr<AST>> & ast) {
    while (!eof()) {
        std::unique_ptr<AST> statement;
        bool success = parseFunction(&statement) || parseStruct(&statement);

        if (!success) {
            if (error.empty()) {
                if (eof()) {
                    std::cerr << "ERR: unexpected end of file " << std::endl;
                } else {
                    auto token = get();
                    std::cerr << "ERR: line " << token.line << ", column " << token.column << std::endl;
                }
            } else {
                std::cerr << "ERR: " << error.message.str() << std::endl;
                std::cerr << "ERR: line " << error.token.line << ", column " << error.token.column << std::endl;
            }
            return ERR_INVALID_TOP_LEVEL_STATEMENT;
        }

        ast.push_back(std::move(statement));
    }

    return ERR_NONE;
}

bool Parser::parseFunction(std::unique_ptr<AST> * statement) {
    auto prototype = parseFunctionPrototype();

    if (prototype == nullptr) {
        return false;
    }

    if (eof()) {
        std::cerr << "ERR: incomplete function prototype" << std::endl;
        return false;
    }

    if (get().type == SEMICOLON) {
        index++;
        
        *statement = std::make_unique<FunctionDeclarationAST>(prototype);
    } else {
        auto body = parseBody();
        if (body == nullptr) {
            std::cerr << "ERR: function \"" << program.extract(prototype->name) << 
                "\" has invalid body" << std::endl;
            return false;
        }

        *statement = std::make_unique<FunctionDefinitionAST>(prototype, body);
    }

    return true;
}

bool Parser::parseStruct(std::unique_ptr<AST> * statement) {
    Token name;
    if (!(expect(STRUCT) && expectIdentifier(&name) && expect(SEMICOLON))) {
        return false;
    }

    *statement = std::make_unique<IncompleteStructAST>(name);
    return true;
}

std::unique_ptr<BodyAST> Parser::parseBody() {
    size_t startIndex = index;

    if (!expect(OPEN_BRACE)) {
        index = startIndex;
        return nullptr;
    }

    auto statements = parseStatementList();

    if (!expect(CLOSE_BRACE)) {
        if (!eof() && error.empty()) {
            error.token = get();
            std::cerr << "missing closing brace, found: \"" <<
                program.extract(error.token) << "\"" << std::endl;
        }
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
    size_t startIndex = index;

    bool requireSemicolon = true;

    std::unique_ptr<AST> statement = parseExpression();
    if (statement == nullptr) {
        statement = parseVariableDefinition();
    }

    if (statement == nullptr) {
        statement = parseWhileLoop();
        requireSemicolon = false;
    }

    if (statement == nullptr) {
        return nullptr;
    }

    if (requireSemicolon && !expect(SEMICOLON)) {
        if (index > 0 && !eof() && error.empty()) {
            error.token = tokens[index - 1];
            error.message << "missing semicolon, found: \"" << program.extract(get()) << "\"";
        }

        index = startIndex;
        return nullptr;
    }

    return statement;
}

std::unique_ptr<VariableDefinitionAST> Parser::parseVariableDefinition() {
    size_t startIndex = index;
    Token name;
    std::unique_ptr<TypeAST> type;
    std::unique_ptr<ExpressionAST> expression;
    bool hasLet = expect(LET);
    bool success = hasLet && 
        expectIdentifier(&name) && expect(COLON) && 
        expectType(&type) && expect(EQUALS) && 
        expectExpression(&expression);

    if (!success) {
        if (!eof() && hasLet) {
            auto token = get();
            std::cerr << "ERR: unexpected symbol in variable definition: \"" << 
                program.extract(token) << "\"" << std::endl;
        }

        index = startIndex;
        return nullptr;
    }

    return std::make_unique<VariableDefinitionAST>(name, type, expression);
}

std::unique_ptr<WhileLoopAST> Parser::parseWhileLoop() {
    size_t startIndex = index;
    std::unique_ptr<ExpressionAST> condition;
    bool hasWhile = expect(WHILE);
    bool success = hasWhile &&
        expect(OPEN_PAREN) && expectExpression(&condition) && expect(CLOSE_PAREN);

    if (!success) {
        if (!eof() && hasWhile) {
            auto token = get();
            std::cerr << "ERR: unexpected symbol in while loop: \"" << 
                program.extract(token) << "\"" << std::endl;
        }

        index = startIndex;
        return nullptr;
    }

    auto body = parseBody();
    if (body == nullptr) {
        if (!eof()) {
            auto token = get();
            std::cerr << "ERR: missing while loop body, found: \"" << 
                program.extract(token) << "\"" << std::endl;
        }

        index = startIndex;
        return nullptr;
    }

    return std::make_unique<WhileLoopAST>(condition, body);
}

std::unique_ptr<ExpressionAST> Parser::parseExpression() {
    std::unique_ptr<ExpressionAST> expr;

    expr = parseNotOperation();
    if (expr) return expr;

    expr = parseToken<IntLiteralAST, INT_LITERAL>();
    if (expr) return expr;
    
    expr = parseToken<StringLiteralAST, STRING_LITERAL>();
    if (expr) return expr;

    expr = parseFunctionCall();
    if (expr) return expr;

    return parseToken<VariableAST, IDENTIFIER>();
}

std::unique_ptr<NotOperationAST> Parser::parseNotOperation() {
    size_t startIndex = index;

    std::unique_ptr<ExpressionAST> expression;
    if (!(expect(NOT) && expectExpression(&expression))) {
        index = startIndex;
        return nullptr;
    }

    return std::make_unique<NotOperationAST>(expression);
}

std::unique_ptr<FunctionCallAST> Parser::parseFunctionCall() {
    // TODO: calling expressions, function pointers

    size_t startIndex = index;

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

        size_t lastIndex = index;
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
    size_t startIndex = index;

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
    case IDENTIFIER:
        index++;
        return std::make_unique<StructTypeAST>(tok);
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
            if (!eof()) {
                auto token = get();
                std::cerr << "ERR: unexpected symbol in pointer type: \"" << 
                    program.extract(token) << "\"" << std::endl;
            }
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
    size_t startIndex = index;

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
        if (!eof()) {
            auto token = get();
            if (token.type != CLOSE_PAREN) {
                std::cerr << "ERR: unexpected symbol in parameter list: \"" << 
                    program.extract(token) << "\"" << std::endl;
            }
        }
        return parameters;
    }

    while (hasParameter) {
        parameters.push_back({ name, std::move(type) });

        size_t lastIndex = index;
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
    *type = parseType();
    return *type != nullptr;
}

bool Parser::expectExpression(std::unique_ptr<ExpressionAST> * expression) {
    assert(expression != nullptr);
    *expression = parseExpression();
    return *expression != nullptr;
}

bool Parser::eof() const {
    return index >= tokens.size();
}

Token Parser::get() const {
    assert(!eof());
    return tokens[index];
}