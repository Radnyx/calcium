#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <map>

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "../include/parser.h"

/*
SIMPLE HELLO WORLD

fun printf(* byte): int;

fun main(): unit {
    let text: * byte = "hello world"; // stored globally, referenced here
    printf(text);
}

GRAPHICS HELLO WORLD

TODO: read https://github.com/KhronosGroup/SPIRV-Guide/tree/master

*/

int main(int, char**) {
    std::string filename = "../../examples/test.ca";
    std::ifstream stream(filename);
    if (!stream) {
        std::cerr << "ERR: could not find file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();

    const std::string program = buffer.str();
    
    // ============ LEXER ============
    
    Lexer lexer(program);
    
    std::vector<Token> tokens;
    auto err = lexer.tokenize(tokens);
    if (err != ERR_NONE) return err;

    // ============ PARSER ============
    
    Parser parser(tokens);

    std::vector<std::unique_ptr<AST>> ast;
    err = parser.parse(ast);

    if (err != ERR_NONE) {
        if (!parser.eof()) {
            auto tok = parser.get();
            std::cerr << "ERR: line " << tok.line << ", column " << tok.column << std::endl;
        }

        return err;
    }

    // ============ SEMANTIC ANALYSIS ============

    // ============ CODE GENERATION ============

    auto llvmContext = std::make_unique<llvm::LLVMContext>();
    auto llvmModule = std::make_unique<llvm::Module>("Calcium", *llvmContext);
    auto irRuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
    std::map<std::string, llvm::Value *> symbols;

    return 0;
}