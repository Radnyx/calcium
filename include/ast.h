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

enum TypeID {
    TYPE_PRIMITIVE,
    TYPE_POINTER,
    TYPE_STRUCT
};

enum ExpressionID {
    EXPRESSION_VARIABLE,
    EXPRESSION_INT_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_FUNCTION_CALL,
    EXPRESSION_NOT_OPERATION,
};

class AST {
public:
    virtual ~AST() = default;
    virtual bool isExpression() const;
    virtual bool isFunctionDeclaration() const;
    virtual bool isFunctionDefinition() const;
};

class BodyAST {
public:
    BodyAST(std::vector<std::unique_ptr<AST>> & statements);
    const std::vector<std::unique_ptr<AST>> statements;
};

class TypeAST {
public:
    virtual ~TypeAST() = default;
    virtual TypeID getTypeID() const = 0;
};

class PrimitiveTypeAST : public TypeAST {
public:
    PrimitiveTypeAST(Primitive primitive);
    const Primitive primitive;
    TypeID getTypeID() const;
};

class PointerTypeAST : public TypeAST {
public:
    PointerTypeAST(std::unique_ptr<TypeAST> & type);
    const std::unique_ptr<TypeAST> type;
    TypeID getTypeID() const;
};

class StructTypeAST : public TypeAST {
public:
    StructTypeAST(Token name);
    const Token name;
    TypeID getTypeID() const;
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
    bool isFunctionDeclaration() const;
};

class FunctionDefinitionAST : public AST {
public:
    FunctionDefinitionAST(std::unique_ptr<FunctionPrototypeAST> & prototype, std::unique_ptr<BodyAST> & body);
    const std::unique_ptr<FunctionPrototypeAST> prototype; 
    const std::unique_ptr<BodyAST> body; 
    bool isFunctionDefinition() const;
};

class IncompleteStructAST : public AST {
public:
    IncompleteStructAST(Token name);
    const Token name;
};


class ExpressionAST : public AST {
public:
    bool isExpression() const;
    virtual ExpressionID getExpressionID() const = 0;
};

class VariableAST : public ExpressionAST {
public:
    VariableAST(Token text);
    const Token text;
    ExpressionID getExpressionID() const;
};

class IntLiteralAST : public ExpressionAST {
public:
    IntLiteralAST(Token text);
    const Token text;
    ExpressionID getExpressionID() const;
};

class StringLiteralAST : public ExpressionAST {
public:
    StringLiteralAST(Token text);
    const Token text;
    ExpressionID getExpressionID() const;
};

class FunctionCallAST : public ExpressionAST {
public:
    FunctionCallAST(Token token, std::vector<std::unique_ptr<ExpressionAST>> & arguments);
    const Token name;
    const std::vector<std::unique_ptr<ExpressionAST>> arguments;
    ExpressionID getExpressionID() const;
};

class NotOperationAST : public ExpressionAST {
public:
    NotOperationAST(std::unique_ptr<ExpressionAST> & expression);
    const std::unique_ptr<ExpressionAST> expression;
    ExpressionID getExpressionID() const;
};

class VariableDefinitionAST : public AST {
public:
    VariableDefinitionAST(Token name, std::unique_ptr<TypeAST> & type, std::unique_ptr<ExpressionAST> & expression);
    const Token name;
    const std::unique_ptr<TypeAST> type;
    const std::unique_ptr<ExpressionAST> expression;
};

class WhileLoopAST : public AST {
public:
    WhileLoopAST(std::unique_ptr<ExpressionAST> & condition, std::unique_ptr<BodyAST> & body);
    const std::unique_ptr<ExpressionAST> condition;
    const std::unique_ptr<BodyAST> body;
};

#endif // AST_H
