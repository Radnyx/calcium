#include "../include/irgenerator.h"
#include "llvm/IR/Constants.h"

static llvm::AllocaInst * createEntryBlockAlloca(
    llvm::Function * function, llvm::Type * type, llvm::StringRef name
) {
    llvm::IRBuilder<> builder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return builder.CreateAlloca(type, nullptr, name);
}

IRGenerator::IRGenerator(
    Program & program,
    std::shared_ptr<llvm::LLVMContext> & llvmContext,
    std::shared_ptr<llvm::Module> & llvmModule
) : program(program), llvmContext(llvmContext), llvmModule(llvmModule) {
    irBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
}

void IRGenerator::generate(const std::vector<std::unique_ptr<AST>> & ast) {
    auto kernelType = llvm::StructType::create(*llvmContext, "Kernel");
    kernelType->setBody({
        irBuilder->getInt8Ty()->getPointerTo(),
        irBuilder->getInt64Ty()
    });

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
        } else if (node->isIncompleteStruct()) {
            auto declaration = static_cast<const IncompleteStructAST *>(node.get());
            incompleteStructs.insert(program.extract(declaration->name));
        } else {
            assert(false);
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
    case PRIMITIVE_BOOL:
        return irBuilder->getInt1Ty();
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
    case TYPE_STRUCT:
    {
        auto structType = static_cast<const StructTypeAST *>(type);
        auto name = program.extract(structType->name);
        if (incompleteStructs.find(name) != incompleteStructs.end()) {
            // can only use as a pointer
            return irBuilder->getInt8Ty();
        }
        return llvm::StructType::getTypeByName(*llvmContext, name);
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
            auto name = arg.getName();
            auto alloc = createEntryBlockAlloca(function, arg.getType(), name);
            irBuilder->CreateStore(&arg, alloc);
            symbols[std::string(arg.getName())].push(alloc);
        }
    }

    generate(definition->body.get());

    // TODO: abstract this and handle explicit return statements
    auto returnType = definition->prototype->returnType.get();
    if (
        returnType->getTypeID() == TYPE_PRIMITIVE && 
        static_cast<const PrimitiveTypeAST *>(returnType)->primitive == PRIMITIVE_UNIT
    ) {
        irBuilder->CreateRet(llvm::ConstantInt::get(irBuilder->getInt1Ty(), 0));
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
        }
        else if (statement->isVariableDefinition()) {
            auto definition = static_cast<const VariableDefinitionAST *>(statement.get());
            auto name = program.extract(definition->name);
            llvm::Function * function = irBuilder->GetInsertBlock()->getParent();
            llvm::Type * type = generate(definition->type.get());
            llvm::Value * value = generate(definition->expression.get());

            auto alloc = createEntryBlockAlloca(function, type, name);
            irBuilder->CreateStore(value, alloc);

            symbols[name].push(alloc); // TODO: pop this when leaving braces scope
        }
        else if (statement->isWhileLoop()) {
            auto whileLoop = static_cast<const WhileLoopAST *>(statement.get());
            llvm::Function * function = irBuilder->GetInsertBlock()->getParent();
            auto conditionBlock = llvm::BasicBlock::Create(*llvmContext, "while.cond", function);
            auto bodyBlock = llvm::BasicBlock::Create(*llvmContext, "while.body", function);
            auto endBlock = llvm::BasicBlock::Create(*llvmContext, "while.end", function);
            irBuilder->CreateBr(conditionBlock);
            // while.cond:
            irBuilder->SetInsertPoint(conditionBlock);
            // compare x <= y
            auto value = generate(whileLoop->condition.get());
            assert(value != nullptr);
            // if true, branch to while.body, otherwise while.end
            irBuilder->CreateCondBr(value, bodyBlock, endBlock);
            // while.body:
            irBuilder->SetInsertPoint(bodyBlock);
            generate(whileLoop->body.get());
            // branch to while.cond
            irBuilder->CreateBr(conditionBlock);
            // while.end:
            irBuilder->SetInsertPoint(endBlock);
        }
        else if (statement->isReturn()) {

        }

    }
}

llvm::Value * IRGenerator::generate(const ExpressionAST * expression) {
    switch (expression->getExpressionID()) {
    case EXPRESSION_INT_LITERAL:
    {
        auto intLiteral = static_cast<const IntLiteralAST *>(expression);
        auto text = program.extract(intLiteral->text);
        return llvm::ConstantInt::get(irBuilder->getInt32Ty(), std::stoi(text));
    }
    case EXPRESSION_STRING_LITERAL:
    {
        auto stringLiteral = static_cast<const StringLiteralAST *>(expression);
        auto text = program.extract(stringLiteral->text);
        return irBuilder->CreateGlobalStringPtr(text);
    }
    case EXPRESSION_VARIABLE:
    {
        auto variable = static_cast<const VariableAST *>(expression);
        auto name = program.extract(variable->text);
        auto alloc = symbols[name].top();
        return irBuilder->CreateLoad(alloc->getAllocatedType(), alloc, name.c_str());
    }
    case EXPRESSION_NOT_OPERATION:
    {
        auto notOperation = static_cast<const NotOperationAST *>(expression);
        auto value = generate(notOperation->expression.get());
        return irBuilder->CreateNot(value, "not");
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