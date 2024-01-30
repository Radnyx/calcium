#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "../include/parser.h"
#include "../include/irgenerator.h"
#include "../include/writer.h"

#include <vulkan/vulkan.h>

/*
compile/link in one go: https://discourse.llvm.org/t/compile-to-native/62196/3

GRAPHICS HELLO WORLD

Vulkan compute:
- https://www.neilhenning.dev/posts/a-simple-vulkan-compute-example/
- https://vulkan-tutorial.com/Compute_Shader

TODO: read https://github.com/KhronosGroup/SPIRV-Guide/tree/master

Refer to https://github.com/KhronosGroup/SPIRV-Tools/blob/main/docs/syntax.md
For an API to generate and assembling SPIR-V code.

*/

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "ERR: expected 1 argument, e,g. calcium main.ca" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream stream(filename);
    if (!stream) {
        std::cerr << "ERR: could not find file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();

    std::string text = buffer.str();
    Program program(text);
    
    // ============ LEXER ============
    
    Lexer lexer(text);
    
    std::vector<Token> tokens;
    auto err = lexer.tokenize(tokens);
    if (err != ERR_NONE) return err;

    // ============ PARSER ============
    
    Parser parser(program, tokens);

    std::vector<std::unique_ptr<AST>> ast;
    err = parser.parse(ast);

    if (err != ERR_NONE) {
        return err;
    }

    // ============ SEMANTIC ANALYSIS ============

    // ============ CODE GENERATION ============

    auto llvmContext = std::make_shared<llvm::LLVMContext>();
    auto llvmModule = std::make_shared<llvm::Module>("Calcium", *llvmContext);

    IRGenerator irGenerator(program, llvmContext, llvmModule);
    irGenerator.generate(ast);

    // ============ OUTPUT TO OBJECT FILE ============

    std::string outputFilename = filename.substr(0, filename.find_last_of(".")) + ".o";

    Writer writer(llvmModule);
    err = writer.output(outputFilename);

    if (err != ERR_NONE) {
        return err;
    }

    std::cout << "INFO: wrote to \"" << outputFilename << "\"" << std::endl;

    return 0;
}