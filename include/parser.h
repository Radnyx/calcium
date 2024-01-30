#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include "errors.h"
#include "lexer.h"
#include "ast.h"

class Parser {
public:
    Parser(const std::vector<Token> & tokens);
    Error parse(std::vector<std::unique_ptr<AST>> & ast);

    bool eof() const;
    Token get() const;
private:
    const std::vector<Token> & tokens;
    size_t index; // current position in the token list

    /* Returns true if it finds the expected syntax, advances the index.
        If false, the caller must keep track of the original index. */
    bool expect(TokenType tokenType);
    bool expectIdentifier(Token * token);
    bool expectType(std::unique_ptr<TypeAST> * type);

    /*On failure, these functions return nullptr and restore the index. */
    std::unique_ptr<BodyAST> parseBody();
    std::vector<std::unique_ptr<AST>> parseStatementList();
    std::unique_ptr<AST> parseStatement();
    std::unique_ptr<ExpressionAST> parseExpression();
    std::unique_ptr<StringLiteralAST> parseStringLiteral();
    std::unique_ptr<FunctionCallAST> parseFunctionCall();
    std::vector<std::unique_ptr<ExpressionAST>> parseExpressionList();

    std::unique_ptr<TypeAST> parseType(); 
    std::unique_ptr<FunctionPrototypeAST> parseFunctionPrototype();
    std::vector<Parameter> parseParameterList();
    bool parseParameter(Token * name, std::unique_ptr<TypeAST> * type);
};

#endif // PARSER_H