#include "../include/irgenerator.h"

IRGenerator::IRGenerator(
    Program & program,
    std::shared_ptr<llvm::LLVMContext> & llvmContext,
    std::shared_ptr<llvm::Module> & llvmModule
) : program(program), llvmContext(llvmContext), llvmModule(llvmModule) {
    irBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
}

void IRGenerator::generate(const std::vector<std::unique_ptr<AST>> & ast) {
    for (auto & node : ast) {
        if (node->isFunctionDeclaration()) {
            auto declaration = static_cast<const FunctionDeclarationAST *>(node.get());
            if (generate(declaration) == nullptr) {
                break;
            }
        } else if (node->isFunctionDefinition()) {
            auto definition = static_cast<const FunctionDefinitionAST *>(node.get());
            if (generate(definition) == nullptr) {
                break;
            }
        } else {
            // ERROR
        }
    }

    llvmModule->print(llvm::errs(), nullptr);
}

llvm::Type * IRGenerator::generate(Primitive primitive) {
    switch (primitive) {
    case PRIMITIVE_UNIT:
        return irBuilder->getInt1Ty();
    case PRIMITIVE_BYTE:
        return irBuilder->getInt8Ty();
    case PRIMITIVE_INT:
        return irBuilder->getInt32Ty();
    }
    return nullptr;
}

llvm::Type * IRGenerator::generate(const TypeAST * type) {
    switch (type->getTypeID()) {
    case TYPE_PRIMITIVE:
    {
        auto primitiveType = static_cast<const PrimitiveTypeAST *>(type);
        return generate(primitiveType->primitive);
    }
    case TYPE_POINTER:
    {
        auto pointerType = static_cast<const PointerTypeAST *>(type);
        auto gen = generate(pointerType->type.get());
        assert(gen != nullptr);

        return gen->getPointerTo();
    }
    default:
        return nullptr;
    }
}

llvm::Function * IRGenerator::generate(const FunctionPrototypeAST * prototype) {
    std::vector<llvm::Type *> paramTypes;
    for (auto & param : prototype->parameters) {
        auto paramType = generate(param.type.get());
        assert(paramType != nullptr);

        paramTypes.push_back(paramType);
    }

    auto returnType = generate(prototype->returnType.get());
    assert(returnType != nullptr);

    auto functionType = llvm::FunctionType::get(returnType, paramTypes, false);
    auto name = program.extract(prototype->name);
    auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, llvmModule.get());
    
    size_t index = 0;
    for (auto & arg : function->args()) {
        auto token = prototype->parameters[index].name;
        if (token.type != NULL_TOKEN) {
            arg.setName(program.extract(token));
        }
        index++;
    }

    return function;
}

llvm::Function * IRGenerator::generate(const FunctionDeclarationAST * declaration) {
    return generate(declaration->prototype.get());
}

llvm::Function * IRGenerator::generate(const FunctionDefinitionAST * definition) {
    std::string name = program.extract(definition->prototype->name);
    llvm::Function * function = llvmModule->getFunction(name);

    if (function == nullptr) {
        function = generate(definition->prototype.get());
    }

    if (function == nullptr) {
        return nullptr;
    }

    llvm::BasicBlock * basicBlock = llvm::BasicBlock::Create(*llvmContext, "entry", function);
    irBuilder->SetInsertPoint(basicBlock);

    for (auto & arg : function->args()) {
        if (arg.hasName()) {
            symbols[std::string(arg.getName())].push(&arg);
        }
    }

    generate(definition->body.get());

    // TODO: abstract this and handle explicit return statements
    auto returnType = definition->prototype->returnType.get();
    if (
        returnType->getTypeID() == TYPE_PRIMITIVE && 
        static_cast<const PrimitiveTypeAST *>(returnType)->primitive == PRIMITIVE_UNIT
    ) {
        irBuilder->CreateRet(llvm::ConstantInt::get(*llvmContext, llvm::APInt(1, false)));
    }

    for (auto & arg : function->args()) {
        if (arg.hasName()) {
            symbols[std::string(arg.getName())].pop();
        }
    }

    return function;
}

void IRGenerator::generate(const BodyAST * body) {
    for (auto & statement : body->statements) {
        if (statement->isExpression()) {
            auto expression = static_cast<const ExpressionAST *>(statement.get());
            auto value = generate(expression);
            if (value == nullptr) {
                return;
            }

            // irBuilder->Insert(value, "tmp");
        }
    }
}

llvm::Value * IRGenerator::generate(const ExpressionAST * expression) {
    switch (expression->getExpressionID()) {
    case EXPRESSION_STRING_LITERAL:
    {
        auto stringLiteral = static_cast<const StringLiteralAST *>(expression);
        auto text = program.extract(stringLiteral->text);
        return irBuilder->CreateGlobalStringPtr(text);
    }
    case EXPRESSION_FUNCTION_CALL:
    {
        auto functionCall = static_cast<const FunctionCallAST *>(expression);
        auto name = program.extract(functionCall->name);
        llvm::Function * callee = llvmModule->getFunction(name);
        if (callee == nullptr) {
            std::cerr << "ERR: attempting to call undefined method \"" << name << "\"" << std::endl;
            return nullptr;
        }

        size_t paramCount = callee->arg_size();
        size_t passedArgCount = functionCall->arguments.size();
        if (paramCount != passedArgCount) {
            std::cerr << "ERR: expected " << paramCount << " arguments passed to \"" 
                      << name << "\", but got " << passedArgCount << std::endl;
            return nullptr;
        }

        std::vector<llvm::Value *> args;
        for (size_t i = 0; i < paramCount; i++) {
            
            auto arg = generate(functionCall->arguments[i].get());
            if (arg == nullptr) {
                return nullptr;
            }

            args.push_back(arg);
        }

        return irBuilder->CreateCall(callee, args, "calltmp");
    }
    default:
        return nullptr;
    }
}