#include "../include/AST.h"

bool AST::isExpression() const {
    return false;
}

bool AST::isFunctionDeclaration() const {
    return false;
}

bool AST::isFunctionDefinition() const {
    return false;
}

bool AST::isIncompleteStruct() const {
    return false;
}

bool AST::isVariableDefinition() const {
    return false;
}

bool AST::isWhileLoop() const {
    return false;
}

bool AST::isReturn() const {
    return false;
}

PrimitiveTypeAST::PrimitiveTypeAST(Primitive primitive) : primitive(primitive) {}

TypeID PrimitiveTypeAST::getTypeID() const {
    return TYPE_PRIMITIVE;
}

PointerTypeAST::PointerTypeAST(std::unique_ptr<TypeAST> & type) : type(std::move(type)) {}

TypeID PointerTypeAST::getTypeID() const {
    return TYPE_POINTER;
}

StructTypeAST::StructTypeAST(Token name) : name(name) {}

TypeID StructTypeAST::getTypeID() const {
    return TYPE_STRUCT;
}


FunctionPrototypeAST::FunctionPrototypeAST(
    Token name,
    std::vector<Parameter> & parameters, 
    std::unique_ptr<TypeAST> & returnType
) 
: name(name), parameters(std::move(parameters)), returnType(std::move(returnType)) {}

FunctionDeclarationAST::FunctionDeclarationAST(std::unique_ptr<FunctionPrototypeAST> & prototype) 
: prototype(std::move(prototype)) {}

bool FunctionDeclarationAST::isFunctionDeclaration() const {
    return true;
}

FunctionDefinitionAST::FunctionDefinitionAST(
    std::unique_ptr<FunctionPrototypeAST> & prototype, 
    std::unique_ptr<BodyAST> & body,
    bool isKernel
)
: prototype(std::move(prototype)), body(std::move(body)), isKernel(isKernel) {}

bool FunctionDefinitionAST::isFunctionDefinition() const {
    return true;
}

IncompleteStructAST::IncompleteStructAST(Token name) : name(name) {}

bool IncompleteStructAST::isIncompleteStruct() const {
    return true;
}

VariableAST::VariableAST(Token text) : text(text) {} 

ExpressionID VariableAST::getExpressionID() const { 
    return EXPRESSION_VARIABLE; 
}

IntLiteralAST::IntLiteralAST(Token text) : text(text) {} 

ExpressionID IntLiteralAST::getExpressionID() const { 
    return EXPRESSION_INT_LITERAL; 
}

FloatLiteralAST::FloatLiteralAST(Token text) : text(text) {} 

ExpressionID FloatLiteralAST::getExpressionID() const { 
    return EXPRESSION_FLOAT_LITERAL; 
}

StringLiteralAST::StringLiteralAST(Token text) : text(text) {} 

ExpressionID StringLiteralAST::getExpressionID() const { 
    return EXPRESSION_STRING_LITERAL; 
}

FunctionCallAST::FunctionCallAST(Token name, std::vector<std::unique_ptr<ExpressionAST>> & arguments)
: name(name), arguments(std::move(arguments)) {}

ExpressionID FunctionCallAST::getExpressionID() const { 
    return EXPRESSION_FUNCTION_CALL; 
}

NotOperationAST::NotOperationAST(std::unique_ptr<ExpressionAST> & expression)
: expression(std::move(expression)) {}

ExpressionID NotOperationAST::getExpressionID() const {
    return EXPRESSION_NOT_OPERATION;
}


VariableDefinitionAST::VariableDefinitionAST(
    Token name, std::unique_ptr<TypeAST> & type, std::unique_ptr<ExpressionAST> & expression
) : name(name), type(std::move(type)), expression(std::move(expression)) {}

bool VariableDefinitionAST::isVariableDefinition() const {
    return true;
}


bool ExpressionAST::isExpression() const {
    return true;
}


WhileLoopAST::WhileLoopAST(std::unique_ptr<ExpressionAST> & condition, std::unique_ptr<BodyAST> & body) 
: condition(std::move(condition)), body(std::move(body)) {}

bool WhileLoopAST::isWhileLoop() const {
    return true;
}


ReturnAST::ReturnAST(std::unique_ptr<ExpressionAST> & expression) : expression(std::move(expression)) {}

bool ReturnAST::isReturn() const {
    return true;
}


BodyAST::BodyAST(std::vector<std::unique_ptr<AST>> & statements) 
: statements(std::move(statements)) {}