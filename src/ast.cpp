#include "../include/ast.h"

PrimitiveTypeAST::PrimitiveTypeAST(Primitive primitive) : primitive(primitive) {}

PointerTypeAST::PointerTypeAST(std::unique_ptr<TypeAST> & type) : type(std::move(type)) {}

FunctionPrototypeAST::FunctionPrototypeAST(
    Token name,
    std::vector<Parameter> & parameters, 
    std::unique_ptr<TypeAST> & returnType
) 
: name(name), parameters(std::move(parameters)), returnType(std::move(returnType)) {}

FunctionDeclarationAST::FunctionDeclarationAST(std::unique_ptr<FunctionPrototypeAST> & prototype) 
: prototype(std::move(prototype)) {}

FunctionDefinitionAST::FunctionDefinitionAST(
    std::unique_ptr<FunctionPrototypeAST> & prototype, 
    std::unique_ptr<BodyAST> & body
)
: prototype(std::move(prototype)), body(std::move(body)) {}

StringLiteralAST::StringLiteralAST(Token literal) : literal(literal) {} 

FunctionCallAST::FunctionCallAST(Token name, std::vector<std::unique_ptr<ExpressionAST>> & arguments)
: name(name), arguments(std::move(arguments)) {}

BodyAST::BodyAST(std::vector<std::unique_ptr<AST>> & statements) 
: statements(std::move(statements)) {}