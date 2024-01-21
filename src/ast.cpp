#include "../include/ast.h"

PrimitiveTypeAST::PrimitiveTypeAST(Primitive primitive) : primitive(primitive) {}

PointerTypeAST::PointerTypeAST(std::unique_ptr<TypeAST> & type) : type(std::move(type)) {}

FunctionPrototypeAST::FunctionPrototypeAST(
    Token name,
    std::vector<Parameter> & parameterList, 
    std::unique_ptr<TypeAST> & returnType
) 
: name(name), parameterList(std::move(parameterList)), returnType(std::move(returnType)) {}