#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include "errors.h"
#include "lexer.h"
#include "ast.h"

class Parser {
public:
    Parser(const std::vector<Token> & tokens);
    Error parse(std::unique_ptr<AST> & ast); 
private:
    const std::vector<Token> & tokens;
    int index; // current position in the token list

    bool eof() const;
    Token get() const;

    bool expect(TokenType tokenType);
    bool expectIdentifier(Token * token);
    bool expectType(std::unique_ptr<TypeAST> * type);

    std::unique_ptr<FunctionPrototypeAST> parseFunctionPrototype();
    bool parseParameter(Token * name, std::unique_ptr<TypeAST> * type);
    std::vector<Parameter> parseParameterList();
    std::unique_ptr<TypeAST> parseType(); // returns nullptr on failure
};

#endif // PARSER_H