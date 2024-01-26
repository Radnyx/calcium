#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H
#include <memory>
#include <map>
#include <stack>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "ast.h"
#include "program.h"

class IRGenerator {
public:
    IRGenerator(
        Program & program,
        std::shared_ptr<llvm::LLVMContext> & llvmContext,
        std::shared_ptr<llvm::Module> & llvmModule
    );

    void generate(const std::vector<std::unique_ptr<AST>> & ast);
    private:
    const Program & program;

    const std::shared_ptr<llvm::LLVMContext> llvmContext;
    const std::shared_ptr<llvm::Module> llvmModule;
    std::unique_ptr<llvm::IRBuilder<>> irBuilder;
    std::map<std::string, std::stack<llvm::Value *>> symbols;

    llvm::Type * generate(Primitive primitive);
    llvm::Type * generate(const TypeAST * type);
    llvm::Function * generate(const FunctionPrototypeAST * prototype);
    llvm::Function * generate(const FunctionDeclarationAST * declaration);
    llvm::Function * generate(const FunctionDefinitionAST * definition);
    void generate(const BodyAST * body);
    llvm::Value * generate(const ExpressionAST * expression);
};

#endif // IR_GENERATOR_H
