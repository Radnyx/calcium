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
    const Token name;
    const std::vector<Parameter> parameterList;
    const std::unique_ptr<TypeAST> returnType;

    FunctionPrototypeAST(
        Token token, 
        std::vector<Parameter> & parameterList, 
        std::unique_ptr<TypeAST> & returnType
    );
};

#endif // AST_H
