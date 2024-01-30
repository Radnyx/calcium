#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include <sstream>
#include "errors.h"
#include "program.h"
#include "ast.h"

struct ParserError {
    Token token;
    std::stringstream message;
    bool empty();
};

class Parser {
public:
    Parser(const Program & program, const std::vector<Token> & tokens);
    Error parse(std::vector<std::unique_ptr<AST>> & ast);

    bool eof() const;
    Token get() const;
private:
    const Program & program;
    const std::vector<Token> & tokens;
    size_t index; // current position in the token list

    ParserError error;

    /* Returns true if it finds the expected syntax, advances the index.
        If false, the caller must keep track of the original index. */
    bool expect(TokenType tokenType);
    bool expectIdentifier(Token * token);
    bool expectType(std::unique_ptr<TypeAST> * type);
    bool expectExpression(std::unique_ptr<ExpressionAST> * expression);

    bool parseFunction(std::unique_ptr<AST> * statement);
    bool parseStruct(std::unique_ptr<AST> * statement);

    /*On failure, these functions return nullptr and restore the index. */
    std::unique_ptr<BodyAST> parseBody();
    std::vector<std::unique_ptr<AST>> parseStatementList();
    std::unique_ptr<AST> parseStatement();
    std::unique_ptr<VariableDefinitionAST> parseVariableDefinition();
    std::unique_ptr<ReturnAST> parseReturn();
    std::unique_ptr<WhileLoopAST> parseWhileLoop();
    std::unique_ptr<ExpressionAST> parseExpression();
    std::unique_ptr<NotOperationAST> parseNotOperation();
    std::unique_ptr<FunctionCallAST> parseFunctionCall();
    std::vector<std::unique_ptr<ExpressionAST>> parseExpressionList();

    std::unique_ptr<TypeAST> parseType(); 
    std::unique_ptr<FunctionPrototypeAST> parseFunctionPrototype(bool isKernel = false);
    std::vector<Parameter> parseParameterList();
    bool parseParameter(Token * name, std::unique_ptr<TypeAST> * type);

    template<typename T, TokenType tokenType>
    std::unique_ptr<T> parseToken() {
        if (eof()) return nullptr;

        Token tok = get();
        if (tok.type == tokenType) {
            index++;
            return std::make_unique<T>(tok);
        }
        return nullptr;
    }
};

#endif // PARSER_H