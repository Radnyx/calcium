#include "../include/Parser.h"
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
    bool isKernel = false;
    auto prototype = parseFunctionPrototype(isKernel);

    if (prototype == nullptr) {
        isKernel = true;
        prototype = parseFunctionPrototype(isKernel);
    }

    if (prototype == nullptr) {
        return false;
    }

    if (eof()) {
        std::cerr << "ERR: incomplete function prototype" << std::endl;
        return false;
    }

    if (get().type == TOK_SEMICOLON) {
        if (isKernel) {
            if (!eof() && error.empty()) {
                error.token = get();
                error.message << "kernel must have function body";
            }
            return false;
        }

        index++;
        
        *statement = std::make_unique<FunctionDeclarationAST>(prototype);
    } else {
        auto body = parseBody();
        if (body == nullptr) {
            std::cerr << "ERR: function \"" << program.extract(prototype->name) << 
                "\" has invalid body" << std::endl;
            return false;
        }

        *statement = std::make_unique<FunctionDefinitionAST>(prototype, body, isKernel);
    }

    return true;
}

bool Parser::parseStruct(std::unique_ptr<AST> * statement) {
    Token name;
    if (!(expect(TOK_STRUCT) && expectIdentifier(&name) && expect(TOK_SEMICOLON))) {
        return false;
    }

    *statement = std::make_unique<IncompleteStructAST>(name);
    return true;
}

std::unique_ptr<BodyAST> Parser::parseBody() {
    size_t startIndex = index;

    if (!expect(TOK_OPEN_BRACE)) {
        index = startIndex;
        return nullptr;
    }

    auto statements = parseStatementList();

    if (!expect(TOK_CLOSE_BRACE)) {
        if (!eof() && error.empty()) {
            error.token = get();
            error.message << "missing closing brace, found: \"" <<
                program.extract(error.token) << "\"";
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
        statement = parseReturn();
    }

    if (statement == nullptr) {
        statement = parseWhileLoop();
        requireSemicolon = false;
    }

    if (statement == nullptr) {
        return nullptr;
    }

    if (requireSemicolon && !expect(TOK_SEMICOLON)) {
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
    bool hasLet = expect(TOK_LET);
    bool success = hasLet && 
        expectIdentifier(&name) && expect(TOK_COLON) && 
        expectType(&type) && expect(TOK_EQUALS) && 
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

std::unique_ptr<ReturnAST> Parser::parseReturn() {
    size_t startIndex = index;
    std::unique_ptr<ExpressionAST> expression;
    bool hasReturn = expect(TOK_RETURN);
    bool success = hasReturn && expectExpression(&expression);
    if (!success) {
        if (!eof() && hasReturn && error.empty()) {
            error.token = get();
            error.message << "missing expression in return statement, found \""
                << program.extract(error.token) << "\"";
        }

        index = startIndex;
        return nullptr;
    }

    return std::make_unique<ReturnAST>(expression);
}

std::unique_ptr<WhileLoopAST> Parser::parseWhileLoop() {
    size_t startIndex = index;
    std::unique_ptr<ExpressionAST> condition;
    bool hasWhile = expect(TOK_WHILE);
    bool success = hasWhile &&
        expect(TOK_OPEN_PAREN) && expectExpression(&condition) && expect(TOK_CLOSE_PAREN);

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

    expr = parseToken<IntLiteralAST, TOK_INT_LITERAL>();
    if (expr) return expr;
    
    expr = parseToken<FloatLiteralAST, TOK_FLOAT_LITERAL>();
    if (expr) return expr;
    
    expr = parseToken<StringLiteralAST, TOK_STRING_LITERAL>();
    if (expr) return expr;

    expr = parseFunctionCall();
    if (expr) return expr;

    return parseToken<VariableAST, TOK_IDENTIFIER>();
}

std::unique_ptr<NotOperationAST> Parser::parseNotOperation() {
    size_t startIndex = index;

    std::unique_ptr<ExpressionAST> expression;
    if (!(expect(TOK_NOT) && expectExpression(&expression))) {
        index = startIndex;
        return nullptr;
    }

    return std::make_unique<NotOperationAST>(expression);
}

std::unique_ptr<FunctionCallAST> Parser::parseFunctionCall() {
    // TODO: calling expressions, function pointers

    size_t startIndex = index;

    Token name;
    if (!(expectIdentifier(&name) && expect(TOK_OPEN_PAREN))) {
        index = startIndex;
        return nullptr;
    }

    auto expressions = parseExpressionList();

    if (!expect(TOK_CLOSE_PAREN)) {
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
        if (!expect(TOK_COMMA)) {
            index = lastIndex;
            break;
        }

        expression = parseExpression();
        // TODO: ALLOWS TRAILING COMMA!
    }

    return expressions;
}

std::unique_ptr<FunctionPrototypeAST> Parser::parseFunctionPrototype(bool isKernel) {
    size_t startIndex = index;

    Token name;
    std::unique_ptr<TypeAST> returnType;

    auto keyword = isKernel ? TOK_KER : TOK_FUN;
    if (!(expect(keyword) && expectIdentifier(&name) && expect(TOK_OPEN_PAREN))) {
        index = startIndex;
        return nullptr;
    }

    auto parameterList = parseParameterList();

    if (!(expect(TOK_CLOSE_PAREN) && expect(TOK_COLON) && expectType(&returnType))) {
        if (!eof() && error.empty()) {
            error.token = get();
            error.message << "unexpected symbol in function prototype: \"" << 
                program.extract(error.token) << "\"";
        }

        index = startIndex;
        return nullptr;
    }

    return std::make_unique<FunctionPrototypeAST>(name, parameterList, returnType);
}

std::unique_ptr<TypeAST> Parser::parseType() {
    if (eof()) return nullptr;

    auto tok = get();
    switch (tok.type) {
    case TOK_IDENTIFIER:
        index++;
        return std::make_unique<StructTypeAST>(tok);
    case TOK_UNIT:
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_UNIT);
    case TOK_BYTE: 
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_BYTE);
    case TOK_INT: 
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_INT);
    case TOK_BOOL: 
        index++;
        return std::make_unique<PrimitiveTypeAST>(PRIMITIVE_BOOL);
    case TOK_STAR:
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

    if (expectIdentifier(name) && expect(TOK_COLON) && expectType(type)) {
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
        if (!eof() && error.empty()) {
            auto token = get();
            if (token.type != TOK_CLOSE_PAREN) {
                error.token = token;
                error.message << "ERR: unexpected symbol in parameter list: \"" << 
                    program.extract(token) << "\"";
            }
        }
        return parameters;
    }

    while (hasParameter) {
        parameters.push_back({ name, std::move(type) });

        size_t lastIndex = index;
        if (!expect(TOK_COMMA)) {
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
    if (tok.type != TOK_IDENTIFIER) return false;
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