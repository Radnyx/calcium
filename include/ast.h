#ifndef AST_H
#define AST_H
#include <memory>
#include <optional>
#include "lexer.h"

enum Primitive {
    PRIMITIVE_UNIT,
    PRIMITIVE_INT,
    PRIMITIVE_BYTE
};

class AST {
public:
    virtual ~AST() = default;
};

class BodyAST {
public:
    BodyAST(std::vector<std::unique_ptr<AST>> & statements);
    const std::vector<std::unique_ptr<AST>> statements;
};

class TypeAST {
public:
    virtual ~TypeAST() = default;
};

class PrimitiveTypeAST : public TypeAST {
public:
    PrimitiveTypeAST(Primitive primitive);
    const Primitive primitive;
};

class PointerTypeAST : public TypeAST {
public:
    PointerTypeAST(std::unique_ptr<TypeAST> & type);
    const std::unique_ptr<TypeAST> type;
};

struct Parameter {
    Parameter() = default;
    Token name;
    std::unique_ptr<TypeAST> type;
};

class FunctionPrototypeAST {
public:
    FunctionPrototypeAST(
        Token token, 
        std::vector<Parameter> & parameters, 
        std::unique_ptr<TypeAST> & returnType
    );
    
    const Token name;
    const std::vector<Parameter> parameters;
    const std::unique_ptr<TypeAST> returnType;
};

class FunctionDeclarationAST : public AST {
public:
    FunctionDeclarationAST(std::unique_ptr<FunctionPrototypeAST> & prototype);
    const std::unique_ptr<FunctionPrototypeAST> prototype; 
};

class FunctionDefinitionAST : public AST {
public:
    FunctionDefinitionAST(std::unique_ptr<FunctionPrototypeAST> & prototype, std::unique_ptr<BodyAST> & body);
    const std::unique_ptr<FunctionPrototypeAST> prototype; 
    const std::unique_ptr<BodyAST> body; 
};

class VariableDefinitionAST : public AST {};

class ExpressionAST : public AST {};

class StringLiteralAST : public ExpressionAST {
public:
    StringLiteralAST(Token token);
    const Token literal;
};

class FunctionCallAST : public ExpressionAST {
public:
    FunctionCallAST(Token token, std::vector<std::unique_ptr<ExpressionAST>> & arguments);
    const Token name;
    const std::vector<std::unique_ptr<ExpressionAST>> arguments;
};

#endif // AST_H
